#include <memory>
#include <boost/format.hpp>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include "gui.hpp"
#include "data_type.hpp"
#include "splitter.hpp"

namespace gui
{
    std::future<int> make_window(image_type const& image, std::string const& window_name)
    {
        splitter sp;

        auto const cloned_image = std::make_shared<unfold_image_type>(sp.unfold_image(image));
        auto const height = image.size();
        auto const width  = image.at(0).size();

        return std::async(
            std::launch::async,
            [=]() -> int
            {
                auto window = std::make_unique<Fl_Window>(width, height, window_name.c_str());
                auto box    = std::make_unique<Fl_Box>(0, 0, width, height);

                Fl_RGB_Image image(cloned_image->data(), width, height);
                box->image(image);

                window->show();
                return Fl::run();
            }
        );
    }

    class MoveBox : public Fl_Box
    {
    public:
        MoveBox(point_type const& point, int const x, int const y, int const w, int const h);
        virtual ~MoveBox() = default;

        point_type const& point() const;

    private:
        void position_swap(MoveBox* lhs, MoveBox* rhs);
        int handle(int const event) override;

        point_type point_;
    };

    class MoveWindow : public Fl_Window
    {
    public:
        MoveWindow(split_image_type const& images, std::string const& window_name);
        virtual ~MoveWindow() = default;

        std::vector<std::vector<point_type>> result() const;

    private:
        splitter sp_;
        std::string const window_name_;
        std::vector<std::vector<unfold_image_type>> image_;
        std::vector<std::vector<Fl_RGB_Image*>> rgbs_;
        std::vector<std::vector<std::unique_ptr<MoveBox>>> boxes_;
    };

    MoveBox::MoveBox(point_type const& point, int const x, int const y, int const w, int const h)
        : Fl_Box(x, y, w, h), point_(point)
    {
    }

    point_type const& MoveBox::point() const
    {
        return this->point_;
    }

    void MoveBox::position_swap(MoveBox* lhs, MoveBox* rhs)
    {
        std::swap(lhs->point_, this->point_);

        int const x = lhs->x();
        int const y = lhs->y();
        lhs->position(rhs->x(), rhs->y());
        rhs->position(x, y);

        lhs->redraw();
        rhs->redraw();
    }

    int MoveBox::handle(int const event)
    {
        int ret = Fl_Box::handle(event);
        switch(event)
        {
            case FL_DND_ENTER:
            {
                std::string text = Fl::event_text();
                if(text.size() == 0) break;

                MoveBox* before = reinterpret_cast<MoveBox*>(std::stoull(text, nullptr, 16));
                if(point_.manhattan(before->point_) == 1) //マンハッタン距離が1なら
                    position_swap(before, this);

                ret = 1;
                break;
            }

            case FL_DND_DRAG:
            case FL_DND_RELEASE:
                ret = 1;
                break;

            case FL_PUSH:
            {
                std::string const msg = (boost::format("0x%016x") % reinterpret_cast<uint64_t>(this)).str();
                Fl::copy(msg.c_str(), msg.size(), 0);
                Fl::dnd();
                ret = 1;
                break;
            }

            case FL_PASTE:
                ret = 1;
                break;
        }

        return ret;
    }

    MoveWindow::MoveWindow(split_image_type const& images, std::string const& window_name)
        : Fl_Window(/*temporary size*/10, 10), window_name_(window_name)
    {
        int const image_width   = images[0][0][0].size();
        int const image_height  = images[0][0]   .size();
        int const split_x       = images[0]      .size();
        int const split_y       = images         .size();
        int const window_width  = split_x * image_width;
        int const window_height = split_y * image_height;

        this->size(window_width, window_height);
        this->label(window_name_.c_str());

        // 引数の「2次元配置された2次元イメージ」を「2次元配置された1次元イメージ」に変える
        image_.clear();
        image_.reserve(split_y);
        for(int i=0; i<split_y; ++i)
        {
            std::vector<unfold_image_type> line;
            line.reserve(split_x);
            for(int j=0; j<split_x; ++j)
            {
                line.push_back(sp_.unfold_image(images[i][j]));
            }
            image_.push_back(std::move(line));
        }

        // 「1次元配置された1次元イメージ」をFLTK用のイメージにリンクさせる．(どちらも生きている必要がある)
        rgbs_.clear();
        rgbs_.reserve(split_y);
        for(int i=0; i<split_y; ++i)
        {
            std::vector<Fl_RGB_Image*> line;
            line.reserve(split_x);
            for(int j=0; j<split_x; ++j)
            {
                line.push_back(new Fl_RGB_Image(image_[i][j].data(), image_width, image_height));
            }
            rgbs_.push_back(std::move(line));
        }

        // FLTKイメージをMoveBoxに関連付ける
        boxes_.clear();
        boxes_.reserve(split_y);
        for(int i=0; i<split_y; ++i)
        {
            std::vector<std::unique_ptr<MoveBox>> line;
            line.reserve(split_x);
            for(int j=0; j<split_x; ++j)
            {
                auto box = std::make_unique<MoveBox>(point_type{j,i}, image_width*j, image_height*i, image_width, image_height);
                box->image(rgbs_[i][j]);
                line.push_back(std::move(box));
            }
            boxes_.push_back(std::move(line));
        }
    }

    std::vector<std::vector<point_type>> MoveWindow::result() const
    {
        int const split_x = image_[0].size();
        int const split_y = image_   .size();
        std::vector<std::vector<point_type>> result(
            split_y,
            std::vector<point_type>(split_x, point_type{-1,-1})
            );

        for(int i=0; i<split_y; ++i)
        {
            for(int j=0; j<split_x; ++j)
            {
                auto const& target = boxes_[i][j]->point();
                result[target.y][target.x] = point_type{j, i};
            }
        }

        return result;
    }
    
    std::future<std::vector<std::vector<point_type>>> make_mansort_window(
        question_raw_data const& data,
        std::string const& window_name
        )
    {
        splitter sp;
        auto splitted = sp.split_image(data);

        return std::async(
            std::launch::async,
            [=, splitted=std::move(splitted)]() -> std::vector<std::vector<point_type>>
            {
                MoveWindow window(splitted, window_name);
                window.show();

                Fl::run();
                return window.result();
            }
        );
    }
} // namespace gui

