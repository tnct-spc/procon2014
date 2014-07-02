#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Box.H>
#include "data_type.hpp"

namespace gui
{
namespace impl
{
    class MoveBox : public Fl_Box
    {
    public:
        MoveBox(point_type const& point, int const x, int const y, int const w, int const h);
        virtual ~MoveBox() = default;

        point_type const& first_point() const;

    private:
        point_type const first_point_;
    };

    class MoveWindow : public Fl_Window
    {
    public:
        MoveWindow(split_image_type const& images, std::string const& window_name);
        virtual ~MoveWindow() = default;

        // 結果を返す関数であるが，期待通りの働きをしない
        std::vector<std::vector<point_type>> result() const;

        // boxes_配列に基づいて，各boxの位置を算出しなおして再描画する
        void boxes_redraw();

        // boxの並び替え操作に関する関数群(内部でredrawするかどうかはランタイム的に検討)
        void box_swap(point_type const& p1, point_type const& p2);
        void box_x_line_swap(int const y1, int const y2);
        void box_y_line_swap(int const x1, int const x2);

        // boxからのイベントを受け取る関数群
        void click_box(int const button, point_type const& event_box);
        void drag_box(point_type const& event_box);
        void release_box(int const button, point_type const& event_box);

        bool border_check(int const x, int const y);          // TrueならWindow内
        point_type window_to_point(int const x, int const y); // Window上の座標から，point_typeを算出
        int handle(int event) override;

    private:
        splitter const sp_;
        std::string const window_name_;

        int const image_width_;
        int const image_height_;
        int const split_x_;
        int const split_y_;

        int latest_button_;
        point_type move_begin_;
        point_type select_begin_;
        point_type select_end_;

        std::vector<std::vector<unfold_image_type>> image_;
        std::vector<std::vector<Fl_RGB_Image*>> rgbs_;
        std::vector<std::vector<std::unique_ptr<MoveBox>>> boxes_;
    };

    MoveBox::MoveBox(point_type const& point, int const x, int const y, int const w, int const h)
        : Fl_Box(x, y, w, h), first_point_(point)
    {
    }

    point_type const& MoveBox::first_point() const
    {
        return first_point_;
    }


    MoveWindow::MoveWindow(split_image_type const& images, std::string const& window_name)
        : Fl_Window(/*temporary size*/10, 10), window_name_(window_name), sp_(), latest_button_(0),
          select_begin_({-1,-1}), select_end_({-1,-1}),
          image_width_ (images[0][0][0].size()),
          image_height_(images[0][0]   .size()),
          split_x_     (images[0]      .size()),
          split_y_     (images         .size())
    {
        int const window_width  = split_x_ * image_width_;
        int const window_height = split_x_ * image_height_;

        this->size(window_width, window_height);
        this->label(window_name_.c_str());

        // 引数の「2次元配置された2次元イメージ」を「2次元配置された1次元イメージ」に変える
        image_.clear();
        image_.reserve(split_y_);
        for(int i=0; i<split_y_; ++i)
        {
            std::vector<unfold_image_type> line;
            line.reserve(split_x_);
            for(int j=0; j<split_x_; ++j)
            {
                line.push_back(sp_.unfold_image(images[i][j]));
            }
            image_.push_back(std::move(line));
        }

        // 「1次元配置された1次元イメージ」をFLTK用のイメージにリンクさせる．(どちらも生きている必要がある)
        rgbs_.clear();
        rgbs_.reserve(split_y_);
        for(int i=0; i<split_y_; ++i)
        {
            std::vector<Fl_RGB_Image*> line;
            line.reserve(split_x_);
            for(int j=0; j<split_x_; ++j)
            {
                line.push_back(new Fl_RGB_Image(image_[i][j].data(), image_width_, image_height_));
            }
            rgbs_.push_back(std::move(line));
        }

        // FLTKイメージをMoveBoxに関連付ける
        boxes_.clear();
        boxes_.reserve(split_y_);
        for(int i=0; i<split_y_; ++i)
        {
            std::vector<std::unique_ptr<MoveBox>> line;
            line.reserve(split_x_);
            for(int j=0; j<split_x_; ++j)
            {
                auto box = std::make_unique<MoveBox>(point_type{j,i}, image_width_*j, image_height_*i, image_width_, image_height_);
                box->image(rgbs_[i][j]);
                line.push_back(std::move(box));
            }
            boxes_.push_back(std::move(line));
        }
    }

    std::vector<std::vector<point_type>> MoveWindow::result() const
    {
        std::vector<std::vector<point_type>> result(
            split_y_,
            std::vector<point_type>(split_x_, point_type{-1,-1})
            );

        for(int i=0; i<split_y_; ++i)
        {
            for(int j=0; j<split_x_; ++j)
            {
                auto const& target = boxes_[i][j]->first_point();
                result[target.y][target.x] = point_type{j, i};
            }
        }

        return result;
    }

    void MoveWindow::boxes_redraw()
    {
        for(int i=0; i<split_y_; ++i)
        {
            for(int j=0; j<split_x_; ++j)
            {
                auto& target_box = boxes_[i][j];
                target_box->resize(image_width_*j, image_height_*i, image_width_, image_height_);
                target_box->redraw();
            }
        }
        return;
    }

    void MoveWindow::box_swap(point_type const& p1, point_type const& p2)
    {
        std::swap(boxes_[p1.y][p1.x], boxes_[p2.y][p2.x]);
    }

    void MoveWindow::box_x_line_swap(int const y1, int const y2)
    {
        std::swap(boxes_[y1], boxes_[y2]);
    }

    void MoveWindow::box_y_line_swap(int const x1, int const x2)
    {
        for(int i=0; i<split_y_; ++i)
        {
            std::swap(boxes_[i][x1], boxes_[i][x2]);
        }
    }

    void MoveWindow::click_box(int const button, point_type const& event_box)
    {
        if(button == FL_LEFT_MOUSE)
        {
            // 通常クリックは交換開始
            move_begin_ = event_box;
        }
        else if(button == FL_MIDDLE_MOUSE)
        {
            // 中クリックは情報クリア
            select_begin_ = select_end_ = point_type{-1,-1};
        }
        else if(button == FL_RIGHT_MOUSE)
        {
            // 右クリックは同時移動ブロックの選択開始
            select_begin_ = event_box;
        }

        latest_button_ = button;
    }

    void MoveWindow::drag_box(point_type const& event_box)
    {
        if(latest_button_ == FL_LEFT_MOUSE)
        {
            // 交換
            if(select_begin_==point_type{-1,-1} || select_end_==point_type{-1,-1})
            {
                // 1つのみ交換
                box_swap(move_begin_, event_box);
                move_begin_ = event_box;
                boxes_redraw();
            }
            else
            {
                // 矩形交換

                // 端チェック
            }
        }
    }

    void MoveWindow::release_box(int const button, point_type const& event_box)
    {
        latest_button_ = 0;
        if(button == FL_RIGHT_MOUSE)
        {
            // 右クリックは同時移動ブロックの選択終了
            select_end_ = event_box;
        }
    }

    bool MoveWindow::border_check(int const x, int const y)
    {
        int const window_width  = image_width_  * split_x_;
        int const window_height = image_height_ * split_y_;

        return (x >= 0 && x < window_width && y >= 0 && y < window_height);
    }

    point_type MoveWindow::window_to_point(int const x, int const y)
    {
        return {x/image_width_, y/image_height_};
    }

    int MoveWindow::handle(int event)
    {
        switch(event)
        {
            case FL_PUSH:
            {
                if(border_check(Fl::event_x(), Fl::event_y()))
                {
                    auto const button = Fl::event_button();
                    auto const point = window_to_point(Fl::event_x(), Fl::event_y());
                    click_box(button, point);
                    return 1;
                }
            }
            case FL_DRAG:
            {
                if(border_check(Fl::event_x(), Fl::event_y()))
                {
                    auto const point = window_to_point(Fl::event_x(), Fl::event_y());
                    drag_box(point);
                    return 1;
                }
            }
            case FL_RELEASE:
            {
                if(border_check(Fl::event_x(), Fl::event_y()))
                {
                    auto const button = Fl::event_button();
                    auto const point = window_to_point(Fl::event_x(), Fl::event_y());
                    release_box(button, point);
                    return 1;
                }
            }
        }

        return 0;
    }

} // namespace impl
} // namespace gui
