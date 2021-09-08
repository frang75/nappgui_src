/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: listboxes.c
 *
 */

/* Listboxes */
    
#include "listboxes.h"
#include "guiall.h"
#include "res.h"

/*---------------------------------------------------------------------------*/

static ListBox *i_full_listbox(void)
{
    ListBox *listbox = listbox_create();
    listbox_size(listbox, s2df(150, 200));
    listbox_multisel(listbox, TRUE);
    listbox_checkbox(listbox, TRUE);
    listbox_add_elem(listbox, "Sales presentation", resid_image(POWERPOINT_PNG));
    listbox_add_elem(listbox, "Balance 2017", resid_image(POWERPOINT_PNG));
    listbox_add_elem(listbox, "The Last of Us Analysis", resid_image(POWERPOINT_PNG));
    listbox_add_elem(listbox, "Phone list", resid_image(ACCESS_PNG));
    listbox_add_elem(listbox, "Customer database", resid_image(ACCESS_PNG));
    listbox_add_elem(listbox, "My first book", resid_image(WORD_PNG));
    listbox_add_elem(listbox, "Letter to April", resid_image(WORD_PNG));
    listbox_add_elem(listbox, "Cookbook Recipes", resid_image(WORD_PNG));
    listbox_add_elem(listbox, "Dog playing piano", resid_image(JPG_PNG));
    listbox_add_elem(listbox, "Hollidays 2019", resid_image(JPG_PNG));
    listbox_add_elem(listbox, "Amanda's party", resid_image(JPG_PNG));
    listbox_add_elem(listbox, "Flying", resid_image(JPG_PNG));
    listbox_add_elem(listbox, "The C Programing Language", resid_image(PDF_PNG));
    listbox_add_elem(listbox, "Graphics Programing with GDI+", resid_image(PDF_PNG));
    listbox_add_elem(listbox, "Personal finances", resid_image(EXCEL_PNG));
    listbox_add_elem(listbox, "Stocks 2017", resid_image(EXCEL_PNG));
    listbox_add_elem(listbox, "Website Dashboard", resid_image(EXCEL_PNG));
    listbox_add_elem(listbox, "Open Issues", resid_image(DOCUMENT_PNG));
    listbox_add_elem(listbox, "TODO List", resid_image(DOCUMENT_PNG));
    listbox_select(listbox, 0, TRUE);
    return listbox;
}

/*---------------------------------------------------------------------------*/

static ListBox *i_image_listbox(void)
{
    ListBox *listbox = listbox_create();
    listbox_size(listbox, s2df(150, 200));
    listbox_add_elem(listbox, "Spain", resid_image(SPAIN_PNG));
    listbox_add_elem(listbox, "Italy", resid_image(ITALY_PNG));
    listbox_add_elem(listbox, "United Kingdom", resid_image(UKING_PNG));
    listbox_add_elem(listbox, "Vietnam", resid_image(VIETNAM_PNG));
    listbox_add_elem(listbox, "Russia", resid_image(RUSSIA_PNG));
    listbox_add_elem(listbox, "Portugal", resid_image(PORTUGAL_PNG));
    listbox_add_elem(listbox, "Japan", resid_image(JAPAN_PNG));
    listbox_add_elem(listbox, "Disk", resid_image(DISK16_PNG));
    listbox_add_elem(listbox, "Edit", resid_image(EDIT16_PNG));
    listbox_add_elem(listbox, "Folder", resid_image(FOLDER16_PNG));
    listbox_add_elem(listbox, "Restore", resid_image(RESTORE16_PNG));
    listbox_add_elem(listbox, "Search", resid_image(SEARCH16_PNG));
    listbox_add_elem(listbox, "Error", resid_image(ERROR16_PNG));
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
    //listbox_add_elem(listbox, "Item 5", NULL);
    //listbox_add_elem(listbox, "Item 6", NULL);
    //listbox_add_elem(listbox, "Item 7", NULL);
    //listbox_add_elem(listbox, "Item 8", NULL);
    //listbox_add_elem(listbox, "Item 9", NULL);
    //listbox_add_elem(listbox, "Item 10", NULL);
    //listbox_add_elem(listbox, "Item 11", NULL);
    //listbox_add_elem(listbox, "Item 12", NULL);
    //listbox_add_elem(listbox, "Item 13", NULL);
    //listbox_add_elem(listbox, "Item 14", NULL);
    //listbox_add_elem(listbox, "Item 15", NULL);
    //listbox_add_elem(listbox, "Item 16", NULL);
    //listbox_add_elem(listbox, "Item 17", NULL);
    //listbox_add_elem(listbox, "Item 18", NULL);
    //listbox_add_elem(listbox, "Item 19", NULL);
    //listbox_add_elem(listbox, "Item 20", NULL);
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
