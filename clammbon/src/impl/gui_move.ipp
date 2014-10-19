#include <mutex>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/range/irange.hpp>
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

    class MoveWindow : public Fl_Window, public boost::enable_shared_from_this<MoveWindow>
    {
    public:
        typedef std::vector<std::vector<point_type>> position_type;

        MoveWindow(split_image_type const& images, position_type const& positions, std::string const& window_name);
        virtual ~MoveWindow() = default;

        void set_data(std::vector<std::vector<point_type>> const& positions);

        // 結果を返す関数であるが，期待通りの働きをしない
        std::vector<std::vector<point_type>> result() const;

        // boxes_配列に基づいて，各boxの位置を算出しなおして再描画する
        void boxes_redraw();

        // boxの並び替え操作に関する関数群
        void box_swap(point_type const& p1, point_type const& p2);
        template<class Range> void range_swap(point_type const& p1, point_type const& p2, Range xrange, Range yrange);

        // boxからのイベントを受け取る関数群
        void click_box(int const button, point_type const& event_box);
        void release_box(int const button, point_type const& event_box);
		void devmatrix(int event_key);

        // TrueならWindow内
        bool border_check(point_type const& point);
        bool border_check(int const x, int const y);

        point_type window_to_point(int const x, int const y); // Window上の座標から，point_typeを算出
        int handle(int event) override;
        void show() override;
        void hide() override;
        bool wait_submit();

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
        std::mutex handler_mutex_;

        std::vector<std::vector<unfold_image_type>> image_;
        std::vector<std::vector<Fl_RGB_Image*>> rgbs_;
        std::vector<std::vector<std::unique_ptr<MoveBox>>> boxes_;

        bool wait_submit_;
        std::vector<std::vector<point_type>> positions_;
        boost::shared_ptr<MoveWindow>        this_shared_;
    };

    MoveBox::MoveBox(point_type const& point, int const x, int const y, int const w, int const h)
        : Fl_Box(x, y, w, h), first_point_(point)
    {
    }

    point_type const& MoveBox::first_point() const
    {
        return first_point_;
    }


    MoveWindow::MoveWindow(split_image_type const& images, position_type const& positions, std::string const& window_name)
        : Fl_Window(/*temporary size*/10, 10), window_name_(window_name), positions_(positions), sp_(), latest_button_(0), this_shared_(nullptr),
          select_begin_({-1,-1}), select_end_({-1,-1}), wait_submit_(false),
          image_width_ (images[0][0].cols),
          image_height_(images[0][0].rows),
          split_x_     (images[0]      .size()),
          split_y_     (images         .size())
    {
        int const window_width  = split_x_ * image_width_;
        int const window_height = split_y_ * image_height_;

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

        boxes_redraw();
    }
    
    void MoveWindow::set_data(std::vector<std::vector<point_type>> const& positions)
    {
        positions_ = positions;
        boxes_redraw();
    }

    std::vector<std::vector<point_type>> MoveWindow::result() const
    {
        return positions_;
    }

    void MoveWindow::boxes_redraw()
    {
        int const pos_width     = positions_.at(0).size();
        int const pos_height    = positions_.size();
        int const window_width  = pos_width * image_width_;
        int const window_height = pos_height * image_height_;
        this->size(window_width, window_height);

        for(int i = 0; i < positions_.size(); ++i)
        {
            for(int j = 0; j < positions_.at(0).size(); ++j)
            {
                if(positions_[i][j] == point_type{-1, -1}) continue; // -1は特別マス

                auto& target_box = boxes_[positions_[i][j].y][positions_[i][j].x];
                target_box->resize(image_width_*j, image_height_*i, image_width_, image_height_);
            }
        }

        redraw();

        return;
    }

    void MoveWindow::box_swap(point_type const& p1, point_type const& p2)
    {
        std::swap(positions_[p1.y][p1.x], positions_[p2.y][p2.x]);
    }
    
    template<class Range>
    void MoveWindow::range_swap(point_type const& p1, point_type const& p2, Range xrange, Range yrange)
    {
        for(auto const y : yrange)
        {
            for(auto const x : xrange)
            {
                point_type const offset = {x, y};
                box_swap(p1 + offset, p2 + offset);
            }
        }
    }

	void MoveWindow::devmatrix(int event_key)
	{
		int const pos_width     = positions_.at(0).size();
		int const pos_height    = positions_.size();
		point_type p1;
		point_type p2;
		if(event_key==FL_Down)
		{
			for (int ypos=pos_height -1; ypos > 0  ; ypos--)
			{
				for (int xpos=0; xpos < pos_width ; xpos++)
				{
					p1.y = ypos;
					p1.x = xpos;
					p2.y = ypos - 1;
					p2.x = xpos;
				box_swap(p1, p2);
				}
			}
			boxes_redraw();
		}
		else if (event_key == FL_Up)
		{
			for (int ypos = 0; ypos < pos_height - 1; ypos++)
			{
				for (int xpos = 0; xpos < pos_width; xpos++)
				{
					p1.y = ypos;
					p1.x = xpos;
					p2.y = ypos + 1;
					p2.x = xpos;
					box_swap(p1, p2);
				}
			}
			boxes_redraw();
		}
		else if (event_key == FL_Left)
		{
			for (int xpos = 0; xpos < pos_width - 1; xpos++)
			{
				for (int ypos = 0; ypos < pos_height; ypos++)
				{
					p1.y = ypos;
					p1.x = xpos;
					p2.y = ypos;
					p2.x = xpos + 1;
					box_swap(p1, p2);
				}
			}
			boxes_redraw();
		}
		else if (event_key == FL_Right)
		{
			for (int xpos = pos_width - 1; xpos > 0; xpos--)
			{
				for (int ypos = 0; ypos < pos_height; ypos++)
				{
					p1.y = ypos;
					p1.x = xpos;
					p2.y = ypos;
					p2.x = xpos - 1;
					box_swap(p1, p2);
				}
			}
			boxes_redraw();
		}
	}

    void MoveWindow::click_box(int const button, point_type const& event_box)
    {
        if(button == FL_LEFT_MOUSE)
        {
            // 通常クリックは交換開始
            move_begin_ = event_box;

            if(!(
                select_begin_.x <= event_box.x && event_box.x <= select_end_.x &&
                select_begin_.y <= event_box.y && event_box.y <= select_end_.y
                ))
            {
                // 選択範囲内の選択でなければ，選択解除
                select_begin_ = select_end_ = point_type{-1,-1};
            }
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
    }

    void MoveWindow::release_box(int const button, point_type const& event_box)
    {
        if(button == FL_LEFT_MOUSE)
        {
            if(select_begin_==point_type{-1,-1} || select_end_==point_type{-1,-1})
            {
                // 単一移動
                box_swap(move_begin_, event_box);
                boxes_redraw();
            }
            else
            {
                // 塊移動
                point_type const direction  = event_box - move_begin_;
                point_type const dest_begin = select_begin_ + direction;
                point_type const dest_end   = select_end_   + direction;
                
                if(!border_check(dest_begin) || !border_check(dest_end))
                {
                    // 移動が範囲外になる
                    return;
                }

                // swap順序を決める
                point_type const select_size = select_end_ - select_begin_;
                auto const xrange = (direction.x > 0)
                    ? boost::irange<int>(select_size.x, -1, -1)
                    : boost::irange<int>(0, select_size.x + 1, 1);
                auto const yrange = (direction.y > 0)
                    ? boost::irange<int>(select_size.y, -1, -1)
                    : boost::irange<int>(0, select_size.y + 1, 1);

                // swap及び，選択範囲の更新
                range_swap(select_begin_, dest_begin, xrange, yrange);
                select_begin_ = dest_begin;
                select_end_   = dest_end;

                boxes_redraw();
            }
        }
        else if(button == FL_RIGHT_MOUSE)
        {
            // 右クリックは同時移動ブロックの選択終了
            select_end_ = event_box;

            if(select_begin_ == select_end_)
            {
                // 1マス範囲選択
                select_begin_ = select_end_ = {-1, -1};
            }
            else
            {
                // 正格化: select_{begin,end}_を，beginが左上になるようにする
                point_type new_begin, new_end;
                new_begin.x = std::min(select_begin_.x, select_end_.x);
                new_begin.y = std::min(select_begin_.y, select_end_.y);
                new_end.x   = std::max(select_begin_.x, select_end_.x);
                new_end.y   = std::max(select_begin_.y, select_end_.y);

                select_begin_ = new_begin;
                select_end_   = new_end;
            }
        }
    }
    
    bool MoveWindow::border_check(point_type const& point)
    {
        int const x_width = positions_.at(0).size();
        int const y_width = positions_      .size();

        return (point.x >= 0 && point.x < x_width && point.y >= 0 && point.y < y_width);
    }

    bool MoveWindow::border_check(int const x, int const y)
    {
        int const window_width  = this->w();
        int const window_height = this->h();

        return (x >= 0 && x < window_width && y >= 0 && y < window_height);
    }

    point_type MoveWindow::window_to_point(int const x, int const y)
    {
        return {x/image_width_, y/image_height_};
    }

    int MoveWindow::handle(int event)
    {
        std::lock_guard<std::mutex> lock(handler_mutex_);
        switch(event)
        {
			case FL_KEYDOWN:
			{
                auto const key = Fl::event_key();
                if(key == FL_Escape)
                {
                    wait_submit_ = true;
                }
                else
                {
                    devmatrix(Fl::event_key());
                }
				return 1;
			}
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
    
    void MoveWindow::show()
    {
        this_shared_ = shared_from_this();
        Fl_Window::show();
    }

    void MoveWindow::hide()
    {
        this_shared_.reset();
        Fl_Window::hide();
    }
    
    bool MoveWindow::wait_submit()
    {
        if(wait_submit_)
        {
            wait_submit_ = false;
            return true;
        }
        else 
            return false;
    }

} // namespace impl
} // namespace gui
