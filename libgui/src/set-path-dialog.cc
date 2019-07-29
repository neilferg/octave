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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <QPushButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QListView>
#include <QFileDialog>
#include <QStatusBar>
#include <QIcon>
#include <QFileInfo>
#include <QTimer>
#include <QDirIterator>
#include <QTextStream>
#include <QGroupBox>
#include <QFileDialog>

#include "set-path-dialog.h"
#include "set-path-model.h"
#include "resource-manager.h"

namespace octave
{
  set_path_dialog::set_path_dialog (QWidget *parent)
    : QDialog (parent)
  {
    setWindowTitle (tr ("Set Path"));

    m_info_label = new QLabel (tr ("All changes take effect immediately."));

    m_add_folder_button = new QPushButton (tr ("Add Folder..."));
    m_move_to_top_button = new QPushButton (tr ("Move to Top"));
    m_move_to_bottom_button = new QPushButton (tr ("Move to Bottom"));
    m_move_up_button = new QPushButton (tr ("Move Up"));
    m_move_down_button = new QPushButton (tr ("Move Down"));
    m_remove_button = new QPushButton (tr ("Remove"));

    m_save_button = new QPushButton (tr ("Save"));
    m_revert_button = new QPushButton (tr ("Revert"));
    m_revert_last_button = new QPushButton (tr ("Revert Last"));

    m_save_button->setFocus ();

    connect (m_add_folder_button, SIGNAL (clicked (void)),
             this, SLOT (add_dir (void)));

    connect (m_remove_button, SIGNAL (clicked (void)),
             this, SLOT (rm_dir (void)));

    connect (m_move_to_top_button, SIGNAL (clicked (void)),
             this, SLOT (move_dir_top (void)));

    connect (m_move_to_bottom_button, SIGNAL (clicked (void)),
             this, SLOT (move_dir_bottom (void)));

    connect (m_move_up_button, SIGNAL (clicked (void)),
             this, SLOT (move_dir_up (void)));

    connect (m_move_down_button, SIGNAL (clicked (void)),
             this, SLOT (move_dir_down (void)));

    set_path_model *model = new set_path_model (this);

    connect (m_save_button, SIGNAL (clicked (void)),
             model, SLOT (save (void)));

    connect (m_revert_button, SIGNAL (clicked (void)),
             model, SLOT (revert (void)));

    connect (m_revert_last_button, SIGNAL (clicked (void)),
             model, SLOT (revert_last (void)));

    m_path_list = new QListView (this);
    m_path_list->setWordWrap (false);
    m_path_list->setModel (model);
    m_path_list->setSelectionBehavior (QAbstractItemView::SelectRows);
    m_path_list->setSelectionMode (QAbstractItemView::ExtendedSelection);
    m_path_list->setAlternatingRowColors (true);

    // layout everything
    QDialogButtonBox *button_box = new QDialogButtonBox (Qt::Horizontal);
    button_box->addButton (m_save_button, QDialogButtonBox::ActionRole);

    // add dialog close button
    m_close_button = button_box->addButton (QDialogButtonBox::Close);
    connect (button_box, SIGNAL (rejected (void)), this, SLOT (close (void)));

    button_box->addButton (m_revert_last_button, QDialogButtonBox::ActionRole);
    button_box->addButton (m_revert_button, QDialogButtonBox::ActionRole);

    // path edit options
    QDialogButtonBox *path_edit_layout = new QDialogButtonBox (Qt::Vertical);
    path_edit_layout->addButton (m_add_folder_button, QDialogButtonBox::ActionRole);
    path_edit_layout->addButton (m_move_to_top_button, QDialogButtonBox::ActionRole);
    path_edit_layout->addButton (m_move_up_button, QDialogButtonBox::ActionRole);
    path_edit_layout->addButton (m_move_down_button, QDialogButtonBox::ActionRole);
    path_edit_layout->addButton (m_move_to_bottom_button, QDialogButtonBox::ActionRole);
    path_edit_layout->addButton (m_remove_button, QDialogButtonBox::ActionRole);

    // main layout
    QHBoxLayout *main_hboxlayout = new QHBoxLayout;
    main_hboxlayout->addWidget(path_edit_layout);
    main_hboxlayout->addWidget(m_path_list);

    QGridLayout *main_layout = new QGridLayout;
    main_layout->addWidget (m_info_label, 0, 0);
    main_layout->addLayout (main_hboxlayout, 1, 0);
    main_layout->addWidget (button_box,2, 0);

    setLayout (main_layout);
  }

  set_path_dialog::~set_path_dialog (void)
  {
  }

  void set_path_dialog::add_dir(void)
  {
    QString dir
      = QFileDialog::getExistingDirectory (this, tr ("Open Directory"),
                                           "",
                                           (QFileDialog::ShowDirsOnly
                                            | QFileDialog::DontResolveSymlinks));
    set_path_model *m = static_cast<set_path_model *> (m_path_list->model ());
    m->add_dir (dir);
  }

  void set_path_dialog::rm_dir (void)
  {
    set_path_model *m = static_cast<set_path_model *> (m_path_list->model ());
    QItemSelectionModel *selmodel = m_path_list->selectionModel ();
    QModelIndexList indexlist = selmodel->selectedIndexes();
    m->rm_dir (indexlist);

    selmodel->clearSelection ();
  }

  void set_path_dialog::move_dir_up (void)
  {
    set_path_model *m = static_cast<set_path_model *> (m_path_list->model ());
    QItemSelectionModel *selmodel = m_path_list->selectionModel ();
    QModelIndexList indexlist = selmodel->selectedIndexes();
    m->move_dir_up (indexlist);

    // Update selection and view
    selmodel->clearSelection ();
    int min_row = m->rowCount () - 1;
    for (int i = 0; i < indexlist.length (); i++)
      {
        int new_row = std::max (indexlist.at (i).row () - 1, 0);
        min_row = std::min (min_row, new_row);
        selmodel->select (m->index (new_row), QItemSelectionModel::Select);
      }

    m_path_list->scrollTo (m->index (min_row));
  }

  void set_path_dialog::move_dir_down (void)
  {
    set_path_model *m = static_cast<set_path_model *> (m_path_list->model ());
    QItemSelectionModel *selmodel = m_path_list->selectionModel ();
    QModelIndexList indexlist = selmodel->selectedIndexes();
    m->move_dir_down (indexlist);

    // Update selection and view
    selmodel->clearSelection ();
    int max_row = 0;
    for (int i = 0; i < indexlist.length (); i++)
      {
        int new_row = std::min (indexlist.at (i).row () + 1, m->rowCount () - 1);
        max_row = std::max (max_row, new_row);
        selmodel->select (m->index (new_row), QItemSelectionModel::Select);
      }

    m_path_list->scrollTo (m->index (max_row));
  }

  void set_path_dialog::move_dir_top (void)
  {
    set_path_model *m = static_cast<set_path_model *> (m_path_list->model ());
    QItemSelectionModel *selmodel = m_path_list->selectionModel ();
    QModelIndexList indexlist = selmodel->selectedIndexes();
    m->move_dir_top (indexlist);

    // Update selection and view
    selmodel->clearSelection ();
    for (int i = 0; i < indexlist.length (); i++)
      selmodel->select (m->index (i), QItemSelectionModel::Select);

    m_path_list->scrollTo (m->index (0));
  }

  void set_path_dialog::move_dir_bottom (void)
  {
    set_path_model *m = static_cast<set_path_model *> (m_path_list->model ());
    QItemSelectionModel *selmodel = m_path_list->selectionModel ();
    QModelIndexList indexlist = selmodel->selectedIndexes();
    m->move_dir_bottom (indexlist);

    // Update selection and view
    selmodel->clearSelection ();
    int row_count = m->rowCount ();
    for (int i = 0; i < indexlist.length (); i++)
      selmodel->select (m->index (row_count - 1 - i),
                        QItemSelectionModel::Select);

    m_path_list->scrollTo (m->index (row_count - 1));
  }

}