/*

Copyright (C) 2019 JunWang

This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/
#if ! defined (octave_set_path_dialog_h)
#define octave_set_path_dialog_h 1

#include <QDialog>
#include <QModelIndex>
#include <QFileInfo>

class QLabel;
class QPushButton;
class QListView;
class QVBoxLayout;
class QHBoxLayout;

namespace octave
{
  class set_path_dialog : public QDialog
  {
    Q_OBJECT

  public:

    set_path_dialog (QWidget *parent = nullptr);

    virtual ~set_path_dialog (void);

  private slots:

    void add_dir(void);

    void rm_dir (void);

    void move_dir_up (void);

    void move_dir_down (void);

    void move_dir_top (void);

    void move_dir_bottom (void);

  private:

    QLabel *m_info_label;
    QPushButton *m_save_button;
    QPushButton *m_close_button;
    QPushButton *m_revert_button;
    QPushButton *m_revert_last_button;

    QListView *m_path_list;

    QPushButton *m_add_folder_button;
    QPushButton *m_move_to_top_button;
    QPushButton *m_move_to_bottom_button;
    QPushButton *m_move_up_button;
    QPushButton *m_move_down_button;
    QPushButton *m_remove_button;
  };
}

#endif