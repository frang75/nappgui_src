/* Listboxes */

#include "listboxes.h"
#include "res_guihello.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

static ListBox *i_full_listbox(void)
{
    ListBox *listbox = listbox_create();
    listbox_size(listbox, s2df(150, 200));
    listbox_multisel(listbox, TRUE);
    listbox_checkbox(listbox, TRUE);
    listbox_add_elem(listbox, "Sales presentation", gui_image(POWERPOINT_PNG));
    listbox_add_elem(listbox, "Balance 2017", gui_image(POWERPOINT_PNG));
    listbox_add_elem(listbox, "The Last of Us Analysis", gui_image(POWERPOINT_PNG));
    listbox_add_elem(listbox, "Phone list", gui_image(ACCESS_PNG));
    listbox_add_elem(listbox, "Customer database", gui_image(ACCESS_PNG));
    listbox_add_elem(listbox, "My first book", gui_image(WORD_PNG));
    listbox_add_elem(listbox, "Letter to April", gui_image(WORD_PNG));
    listbox_add_elem(listbox, "Cookbook Recipes", gui_image(WORD_PNG));
    listbox_add_elem(listbox, "Dog playing piano", gui_image(JPG_PNG));
    listbox_add_elem(listbox, "Hollidays 2019", gui_image(JPG_PNG));
    listbox_add_elem(listbox, "Amanda's party", gui_image(JPG_PNG));
    listbox_add_elem(listbox, "Flying", gui_image(JPG_PNG));
    listbox_add_elem(listbox, "The C Programing Language", gui_image(PDF_PNG));
    listbox_add_elem(listbox, "Graphics Programing with GDI+", gui_image(PDF_PNG));
    listbox_add_elem(listbox, "Personal finances", gui_image(EXCEL_PNG));
    listbox_add_elem(listbox, "Stocks 2017", gui_image(EXCEL_PNG));
    listbox_add_elem(listbox, "Website Dashboard", gui_image(EXCEL_PNG));
    listbox_add_elem(listbox, "Open Issues", gui_image(DOCUMENT_PNG));
    listbox_add_elem(listbox, "TODO List", gui_image(DOCUMENT_PNG));
    listbox_select(listbox, 0, TRUE);
    return listbox;
}

/*---------------------------------------------------------------------------*/

static ListBox *i_image_listbox(void)
{
    ListBox *listbox = listbox_create();
    listbox_size(listbox, s2df(150, 200));
    listbox_add_elem(listbox, "Spain", gui_image(SPAIN_PNG));
    listbox_add_elem(listbox, "Italy", gui_image(ITALY_PNG));
    listbox_add_elem(listbox, "United Kingdom", gui_image(UKING_PNG));
    listbox_add_elem(listbox, "Vietnam", gui_image(VIETNAM_PNG));
    listbox_add_elem(listbox, "Russia", gui_image(RUSSIA_PNG));
    listbox_add_elem(listbox, "Portugal", gui_image(PORTUGAL_PNG));
    listbox_add_elem(listbox, "Japan", gui_image(JAPAN_PNG));
    listbox_add_elem(listbox, "Disk", gui_image(DISK16_PNG));
    listbox_add_elem(listbox, "Edit", gui_image(EDIT16_PNG));
    listbox_add_elem(listbox, "Folder", gui_image(FOLDER16_PNG));
    listbox_add_elem(listbox, "Restore", gui_image(RESTORE16_PNG));
    listbox_add_elem(listbox, "Search", gui_image(SEARCH16_PNG));
    listbox_add_elem(listbox, "Error", gui_image(ERROR16_PNG));
    listbox_select(listbox, 0, TRUE);
    return listbox;
}

/*---------------------------------------------------------------------------*/

static ListBox *i_simple_listbox(void)
{
    ListBox *listbox = listbox_create();
    listbox_size(listbox, s2df(150, 200));
    listbox_add_elem(listbox, "Item 1", NULL);
    listbox_add_elem(listbox, "Item 2", NULL);
    listbox_add_elem(listbox, "Item 3", NULL);
    listbox_add_elem(listbox, "Item 4", NULL);
    listbox_color(listbox, 0, gui_alt_color(color_rgb(128, 0, 0), kCOLOR_RED));
    listbox_color(listbox, 1, gui_alt_color(color_rgb(0, 128, 0), kCOLOR_GREEN));
    listbox_color(listbox, 2, gui_alt_color(color_rgb(0, 0, 128), kCOLOR_BLUE));
    listbox_select(listbox, 0, TRUE);
    return listbox;
}

/*---------------------------------------------------------------------------*/

Panel *listboxes(void)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(3, 2);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    ListBox *listbox1 = i_simple_listbox();
    ListBox *listbox2 = i_image_listbox();
    ListBox *listbox3 = i_full_listbox();
    label_text(label1, "Simple ListBox");
    label_text(label2, "With Images");
    label_text(label3, "Checks and Multiselect");
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 1, 0);
    layout_label(layout, label3, 2, 0);
    layout_listbox(layout, listbox1, 0, 1);
    layout_listbox(layout, listbox2, 1, 1);
    layout_listbox(layout, listbox3, 2, 1);
    layout_hmargin(layout, 0, 10);
    layout_hmargin(layout, 1, 10);
    layout_vmargin(layout, 0, 5);
    panel_layout(panel, layout);
    return panel;
}
