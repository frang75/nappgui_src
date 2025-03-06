# NAppGUI Changelog

## v1.5.1 - WIP

### Added

- `window_is_visible()`. [Issue](https://github.com/frang75/nappgui_src/issues/173). [Commit](https://github.com/frang75/nappgui_src/commit/a415b4f3db1abd6d147f5a35d103debb6973e629).
- `ekGUI_SCALE_ADJUST` to `gui_scale_t`, for `imageview_scale()`. [Commit](https://github.com/frang75/nappgui_src/commit/814e28b45ac1cbf91b70de412f55724b819e158a).
- `button_min_width()`. [Commit](https://github.com/frang75/nappgui_src/commit/8639d19df195376620179e17f523646242a5e823).
- `edit_min_width()`. [Commit](https://github.com/frang75/nappgui_src/commit/8639d19df195376620179e17f523646242a5e823).
- `edit_min_height()`. [Commit]().
- `button_get_font()`. [Commit](https://github.com/frang75/nappgui_src/commit/8639d19df195376620179e17f523646242a5e823).
- `label_min_width()`. [Commit](https://github.com/frang75/nappgui_src/commit/bccc5d36a6ec9f68b0f3e682067da456b2b98ba4).
- `label_get_font()`. [Commit](https://github.com/frang75/nappgui_src/commit/bccc5d36a6ec9f68b0f3e682067da456b2b98ba4).
- `progress_min_width()`. [Commit](https://github.com/frang75/nappgui_src/commit/bccc5d36a6ec9f68b0f3e682067da456b2b98ba4).
- `slider_min_width()`. [Commit](https://github.com/frang75/nappgui_src/commit/bccc5d36a6ec9f68b0f3e682067da456b2b98ba4).
- Dynamic menus. [Doc](https://nappgui.com/en/gui/menu.html). [Issue](https://github.com/frang75/nappgui_src/issues/174). [Commit](https://github.com/frang75/nappgui_src/commit/bd9fb788c687bd7a6c31bf115cd9a82e2b87ffe2).
    * Renamed `menu_add_item()`, previously `menu_item()`.
    * Renamed `menu_count()`, previously `menu_size()`.
    * Removed `menu_hide()`.
    * Added `menu_ins_item()`.
    * Added `menu_del_item()`.
    * Added `menu_get_citem()`.
    * Added `menu_is_menubar()`.
    * Added `menuitem_get_text()`.
    * Added `menuitem_get_image()`.
    * Added `menuitem_get_separator()`.
    * Added `menuitem_get_enabled()`.
    * Added `menuitem_get_visible()`.
    * Added `menuitem_get_state()`.
- `edit_OnChange()` set `cpos` and `len` values in `EvText` event parameters. [Commit]().
- `b64_encode_from_cstr()`. [Commit]().

### Changed

- `label_multiline()` in now a setter and not a constructor. [Commit](https://github.com/frang75/nappgui_src/commit/bccc5d36a6ec9f68b0f3e682067da456b2b98ba4).

### Fixed

- `popup_clear()` issue in macOS. [Issue](https://github.com/frang75/nappgui_src/issues/123). [Commit](https://github.com/frang75/nappgui_src/commit/b61c5cdda50b1587646c6b2fa7df3a99b8a25741).
- `json_write()` issue with `null` values. [Issue](https://github.com/frang75/nappgui_src/issues/175). [Commit](https://github.com/frang75/nappgui_src/commit/d2baa3f2823aeebf554f7492cad205b7ca29a7ca).
- Issue in NRC generated string with new lines. [Issue](https://github.com/frang75/nappgui_src/issues/177). [Commit](https://github.com/frang75/nappgui_src/commit/c4359a9ff45e5c54547048f1046d64d5f22e987a).
- Issue in `str_relpath()`. [Commit](https://github.com/frang75/nappgui_src/commit/b34b6832c361ada1acc3a6a3235ea2fa65a71d88).
- `image_write()` in macOS, with indexed BMPs. [Commit](https://github.com/frang75/nappgui_src/commit/4bc8cd66ff0d4d9a341ad4a7cd17604cc36827f3).
- Issue in `dbind()` default values in struct types. [Commit](https://github.com/frang75/nappgui_src/commit/73a8e0775588edd053ba707c8a748ce2b0b8c9e7).
- Avoid GTK Warnings in `edit_OnFilter()` events with multiline controls. [Issue](https://github.com/frang75/nappgui_src/issues/187). [Commit]().
- Excessive internal padding in GTK `button_flat()`. [Commit]().

### Build system

- Updated to macOS Sequoia. [Commit](https://github.com/frang75/nappgui_src/commit/a524558a4267bd53f7853ce7161846cf42a75361).

## v1.5.0 - Jan 1, 2025 (r5810)

### Added

- `.clang-format` file. [Issue](https://github.com/frang75/nappgui_src/issues/161). [Commit](https://github.com/frang75/nappgui_src/commit/40356b8b17a7cb1f39c2bdfa3c7d4e8ce6ef35d8).
- `arrst_insert_n0()`. [Doc](https://nappgui.com/en/core/arrst.html#f24). [Commit](https://github.com/frang75/nappgui_src/commit/50887ca89b6a844f6b683ca1d2937f3a03cfb004).
- `listbox_del_elem()`. [Doc](https://nappgui.com/en/gui/listbox.html#f9). [Commit](https://github.com/frang75/nappgui_src/commit/50887ca89b6a844f6b683ca1d2937f3a03cfb004).
- `bfile_rename()`. [Doc](https://nappgui.com/en/osbs/bfile.html#f21). [Commit](https://github.com/frang75/nappgui_src/commit/50887ca89b6a844f6b683ca1d2937f3a03cfb004).
- Improved support for data binding [Doc](https://nappgui.com/en/core/dbind.html). [Commit](https://github.com/frang75/nappgui_src/commit/ed4d43572d6c93ca6ee1ddfbe00d8640a519fa76).
    * Added `core::dbindst_t` result type.
    * `dbind()` returns now `dbindst_t`.
    * `dbind_enum()` returns now `dbindst_t`.
    * Added `dbind_binary()`.
    * Added `dbind_alias()`.
    * Added `dbind_unreg()`.
    * Added `dbind_copy()`.
    * Added `dbind_cmp()`.
    * Added `dbind_equ()`.
    * Added `json_read_str()`.
    * Added `json_write_str()`.
    * Added `buffer_read()`.
    * Added `buffer_write()`.
    * Added `JsonOpts` new field `ArrPt(String) *log`.
- `bfile_dir_tmp()`. [Commit](https://github.com/frang75/nappgui_src/commit/f7b5470cbcabeb7bceb27268f718ccd118a7b9c0).
- `hfile_tmp_path()`. [Commit](https://github.com/frang75/nappgui_src/commit/f7b5470cbcabeb7bceb27268f718ccd118a7b9c0).
- `b64_encode_from_stm`. [Commit](https://github.com/frang75/nappgui_src/commit/f7b5470cbcabeb7bceb27268f718ccd118a7b9c0).
- `b64_encode_from_file`. [Commit](https://github.com/frang75/nappgui_src/commit/f7b5470cbcabeb7bceb27268f718ccd118a7b9c0).
- `b64_decode_from_str`. [Commit](https://github.com/frang75/nappgui_src/commit/f7b5470cbcabeb7bceb27268f718ccd118a7b9c0).
- `b64_decode_from_data`. [Commit](https://github.com/frang75/nappgui_src/commit/f7b5470cbcabeb7bceb27268f718ccd118a7b9c0).
- `b64_encode_from_data`. [Commit](https://github.com/frang75/nappgui_src/commit/f7b5470cbcabeb7bceb27268f718ccd118a7b9c0).

### Changed

- `SetSt()`, `SetPt()` use compare-key instead compare-objects. [Commit](https://github.com/frang75/nappgui_src/commit/ed4d43572d6c93ca6ee1ddfbe00d8640a519fa76).

### Fixed

- Issue in `Layout` when window becomes very small. [Commit](https://github.com/frang75/nappgui_src/commit/3d616fa82e072b6c46f4cf196df0516912ab634c).
- Issue in `bmath_prec()`. [Commit](https://github.com/frang75/nappgui_src/commit/062d2a69ea187c2c983b0db02f022a7552295a19).
- Vulnerability in `str_upd()`. [Commit](https://github.com/frang75/nappgui_src/commit/50887ca89b6a844f6b683ca1d2937f3a03cfb004).
- Issue in `dlib_open()` in macOS. [Issue](https://github.com/frang75/nappgui_src/issues/164). [Commit](https://github.com/frang75/nappgui_src/commit/393f098c8153dba0282546cb48b47b4dbe4fc217).
- Issue in `heap_realloc_n()` macro parenthesis. [Issue](https://github.com/frang75/nappgui_src/issues/166). [Commit](https://github.com/frang75/nappgui_src/commit/7653f529f15d8e748d47ef3772ac349ddd6f4231).

### Improved

- Refactor code using `cast()` macro for pointer conversion. [Commit](https://github.com/frang75/nappgui_src/commit/569e797f74f94c54b81d6b7f5f3bd56e43f812c0), [Commit](https://github.com/frang75/nappgui_src/commit/79c15ce90743d9f58488e10d41a992613ebb092e), [Commit](https://github.com/frang75/nappgui_src/commit/dd459fbe248735fef5ba08f71ca77325b55e5bde).

### Build system

- Avoid NAppGUI library paths used as global headers search path. [Issue](https://github.com/frang75/nappgui_src/issues/153). [Commit](https://github.com/frang75/nappgui_src/commit/07b481226c16974707b83ecad6ac5554af5b2f34).

## v1.4.3 - Oct 12, 2024 (r5533)

### Added

* macOS support for `edit_select()`. [Commit](https://github.com/frang75/nappgui_src/commit/d54bdc23f73800817975a840d3d927f00b720452).
* macOS support for `edit_copy()`, `edit_paste()` and `edit_cut()`. [Commit](https://github.com/frang75/nappgui_src/commit/63e667c951ff4850b25de073643c9bdda7069e5e).
* New demo on text selection and clipboard. [Doc](https://nappgui.com/en/howto/guihello.html#h5). [Commit](https://github.com/frang75/nappgui_src/commit/96f6fc272850376103050f283171d60da1f0c00b).
* macOS support for `textview_select()`. [Commit](https://github.com/frang75/nappgui_src/commit/f6f7ca5a2adbab9119c4c3b168747738ad40a96d).
* macOS support for `textview_copy()`, `textview_paste()` and `textview_cut()`. [Commit](https://github.com/frang75/nappgui_src/commit/f6f7ca5a2adbab9119c4c3b168747738ad40a96d).
* macOS support for `textview_OnFilter()`. [Commit](https://github.com/frang75/nappgui_src/commit/f6f7ca5a2adbab9119c4c3b168747738ad40a96d).
* `unicode_move()`. [Doc](https://nappgui.com/en/sewer/unicode.html#f15). [Commit](https://github.com/frang75/nappgui_src/commit/f6f7ca5a2adbab9119c4c3b168747738ad40a96d).
* Button shortcuts. [Doc](https://nappgui.com/en/gui/button.html#h2). [Commit](https://github.com/frang75/nappgui_src/commit/199ffb74dd61105ae6ee541d6c71d21d1fe1d328).
* `textview_wrap()`. [Doc](https://nappgui.com/en/gui/textview.html#h4). [Issue](https://github.com/frang75/nappgui_src/issues/97). [Commit](https://github.com/frang75/nappgui_src/commit/25ea6095a5143249de454379cf222f06610bb258).
* `view_allow_tab()`. [Doc](https://nappgui.com/en/gui/view.html#f24). [Commit](https://github.com/frang75/nappgui_src/commit/811c72ebe9d198e87398c7060628580f81cb8573).
* `textview_apply_all()`, `textview_apply_sel()`. [Doc](https://nappgui.com/en/gui/textview.html#h4). [Issue](https://github.com/frang75/nappgui_src/issues/147). [Commit](https://github.com/frang75/nappgui_src/commit/f6c2ee06145f6f8c830c1e3870a1ecb6ff517b36).
* `textview_show_select()`. [Commit](https://github.com/frang75/nappgui_src/commit/f6c2ee06145f6f8c830c1e3870a1ecb6ff517b36).
* New TextEditor demo. [Doc](https://nappgui.com/en/howto/guihello.html#h6). [Commit](https://github.com/frang75/nappgui_src/commit/f6c2ee06145f6f8c830c1e3870a1ecb6ff517b36).
* Support for OpenGL 3D graphics. [Issue](https://github.com/frang75/nappgui_src/issues/84). [Commit](https://github.com/frang75/nappgui_src/commit/241be10f0258ff2ecaf2087a3a033876aa4b58cd).
    * New library `OGL3D`.
    * OpenGL context `<ogl3d/ogl3d.h>`.
    * [Documentation](https://nappgui.com/en/ogl3d/ogl3d.html).
    * [Demo](https://nappgui.com/en/howto/glhello.html).
* `draw_r2df()`, `draw_r2dd()`, `Draw::r2d()`. [Commit](https://github.com/frang75/nappgui_src/commit/51d4022b5d7535f325b253a29e653b9b88b5ced7).
* `draw2d_preferred_monospace()`. [Commit](https://github.com/frang75/nappgui_src/commit/3e0bb15a8244d8d5e21a055148deda48ab09c74b).
* `listbox_font()`. [Commit](https://github.com/frang75/nappgui_src/commit/3e0bb15a8244d8d5e21a055148deda48ab09c74b).
* `ekFCELL` in font style to indicate the font size means cell height. [Commit](https://github.com/frang75/nappgui_src/commit/3e0bb15a8244d8d5e21a055148deda48ab09c74b).
* Support for font stretch. [Doc](https://nappgui.com/en/draw2d/font.html#h6). [Commit](https://github.com/frang75/nappgui_src/commit/3e0bb15a8244d8d5e21a055148deda48ab09c74b).
    * `font_with_width()`.
    * `font_with_xscale()`.
    * `font_width()`.
    * `font_xscale()`.

### Fixed

* GTK scrollbars position. [Commit](https://github.com/frang75/nappgui_src/commit/5c41f9697ea7e6664b8b9db6cf16c32229ea8c36).
* GTK flatbutton padding. [Commit](https://github.com/frang75/nappgui_src/commit/5c41f9697ea7e6664b8b9db6cf16c32229ea8c36).
* GTK render listbox checks in Xubuntu. [Commit](https://github.com/frang75/nappgui_src/commit/3c6247159f311195b2871b1fa10492c986b4f42d).
* macOS HighSierra and lowers focus ring drawing issue. [Commit](https://github.com/frang75/nappgui_src/commit/a163cb2555101b831414b6deb781a1d1c49ccd42).
* Issue in `dbind_destroy()`. [Commit](https://github.com/frang75/nappgui_src/commit/f8e16ad9d7712339400ca55b1a3ab4a426f1da2a).
* Issue in `layout_panel_replace()`. [Commit](https://github.com/frang75/nappgui_src/commit/f8e16ad9d7712339400ca55b1a3ab4a426f1da2a).
* GTK buttons always show shortcut underscore. [Commit](https://github.com/frang75/nappgui_src/commit/3e0bb15a8244d8d5e21a055148deda48ab09c74b).
* macOS pushbuttons images were not shown. [Commit](https://github.com/frang75/nappgui_src/commit/3e0bb15a8244d8d5e21a055148deda48ab09c74b).
* `gui_alt_color()` avoid to register twice the same color. [Commit](https://github.com/frang75/nappgui_src/commit/3e0bb15a8244d8d5e21a055148deda48ab09c74b).

### Improved

* NAppGUI build documentation. [Doc](https://nappgui.com/en/guide/build.html).

### Build system

* CMake install use `NAppGUIOptions.txt` for build info. [Commit](https://github.com/frang75/nappgui_src/commit/dcbdbcd38926450a7ffd259604e591190e77b377).
* MinGW desktop apps don't require a local `Application.manifest`. They reference the same file in `/prj/templates`. [Commit](https://github.com/frang75/nappgui_src/commit/ff6a60fab7333bb721c55e5000268107758813f4).
* New function `nap_link_inet(targetName)`. [Commit](https://github.com/frang75/nappgui_src/commit/241be10f0258ff2ecaf2087a3a033876aa4b58cd).
* New function `nap_link_opengl(targetName)`. [Commit](https://github.com/frang75/nappgui_src/commit/241be10f0258ff2ecaf2087a3a033876aa4b58cd).
* New CMake option `-DNAPPGUI_NO_CRTDBG=YES` to disable the CRT debugger in Windows. [Issue](https://github.com/frang75/nappgui_src/issues/152). [Commit](https://github.com/frang75/nappgui_src/commit/b65fc3194b22b77bab00ba079e25de96a4d3accc).

## v1.4.2 - Jun 30, 2024 (r5177)

### Added

* `osapp_argc()` and `osapp_argv()`. [Commit](https://github.com/frang75/nappgui_src/commit/10a8dbc659bc270ed6811ff92926d1f0b388818f).
* Improved font monospace support and metrics. [Doc](https://nappgui.com/en/draw2d/font.html#h5). [Commit](https://github.com/frang75/nappgui_src/commit/e3697de2eb0e83da8946bda33ce11fe8ea588618).
    * `font_ascent()`.
    * `font_descent()`.
    * `font_leading()`.
    * `font_is_monospace()`.
    * `font_installed_monospace()`.
* WebView support. [Doc](https://nappgui.com/en/gui/webview.html). [Issue](https://github.com/frang75/nappgui_src/issues/83). [Commit](https://github.com/frang75/nappgui_src/commit/586a6a156a222c5a30fa03e41efe782f08e898d2).
    * New public header `webview.h`.
    * `guicontrol_webview()`.
    * `layout_webview()`.
    * `layout_get_webview()`.
    * `cell_webview()`.
* `blib_setenv()` [Doc](https://nappgui.com/en/sewer/blib.html#f18). [Commit](https://github.com/frang75/nappgui_src/commit/d7e68115418498eea0f8f48cc6966bece5b731f6).

### Fixed

* WindowsXP TableView/ListBox select colors. [Commit](https://github.com/frang75/nappgui_src/commit/fd0983ff06cbea190a543e9f119e749c436b954d).
* macOS TableView/ListBox select colors. [Issue](https://github.com/frang75/nappgui_src/issues/115). [Commit](https://github.com/frang75/nappgui_src/commit/7485964f68e5647c5a31e267ce3a8d2b059ca031).
* WindowsXP flat buttons drawing. [Commit](https://github.com/frang75/nappgui_src/commit/adbb2db0f614db810a17f6c945a44134294efa60).
* macOS Snow Leopard focus ring. [Commit](https://github.com/frang75/nappgui_src/commit/0bd24f6dcb2135e8de763a8a98dd64dc7c98d6c6).
* Crash in ColorView demo. [Issue](https://github.com/frang75/nappgui_src/issues/131). [Commit](https://github.com/frang75/nappgui_src/commit/761ce9ab81a33f74f7591609f02508d5da2c1dc7).
* Crash in `str_repl()` on macOS.[Doc](https://nappgui.com/en/core/string.html#f13).
* GTK `window_OnClose()` doesn't hide the window when `FALSE` is returned. [Commit](https://github.com/frang75/nappgui_src/commit/a2dec614102697283d9a80cbd9e9e132fee17dae).

### Removed

* `guicontrol_enabled()` and `guicontrol_visible()` because unsafe. Use `cell_enabled()`, `cell_visible()`. [Commit](https://github.com/frang75/nappgui_src/commit/8b1fecce6a44adffc827c1fd55fbd86d72f71ad9).

### Improved

* `gui_text_t` properties in private API. [Commit](https://github.com/frang75/nappgui_src/commit/579766ab451eb445ce5013a93892261997137706).
* `str_splits()` adds a new `add_empty` parameter. [Issue](https://github.com/frang75/nappgui_src/issues/124). [Doc](https://nappgui.com/en/core/string.html#f52). [Commit](https://github.com/frang75/nappgui_src/commit/845c4c01ca282abbe6751ed51cb6a28b1860076f).

### Build system

* macOS Snow Leopard/Lion compiler warnings. [Commit](https://github.com/frang75/nappgui_src/commit/66a2305261121dacd7449d4f689ca18a14fc1dc0).
* Support for Ubuntu 24.04 LTS (gcc-13, clang-18).
* Support for MinGW. [Doc](https://nappgui.com/en/guide/win_mac_linux.html#h4). [Commit](https://github.com/frang75/nappgui_src/commit/5f638074231f028a5eb8a07c2f0d232edad408c6). [Issue](https://github.com/frang75/nappgui_src/issues/125). [Issue](https://github.com/frang75/nappgui_src/issues/48). [Issue](https://github.com/frang75/nappgui_src/issues/5). [Discussion](https://github.com/frang75/nappgui_src/discussions/111).
* Avoid warnings using modern C/C++ standards. [Issue](https://github.com/frang75/nappgui_src/issues/137). [Commit](https://github.com/frang75/nappgui_src/commit/86265ffe3c7c933d09b179d431d0025ec2985ddc).
* Avoid CMake warnings with latest version. [Issue](https://github.com/frang75/nappgui_src/issues/119). [Issue](https://github.com/frang75/nappgui_src/issues/118). [Commit](https://github.com/frang75/nappgui_src/commit/cda8a3fccf101ecae03a383fd481517518d70075).

## v1.4.1 - Apr 3, 2024 (r4993)

### Added

* `unicode_convers_nbytes_n()`. [Commit](https://github.com/frang75/nappgui_src/commit/9df21d8294237acb47b6c088a57e58f43ebdfbd8).
* `popup_get_text()`. [Commit](https://github.com/frang75/nappgui_src/commit/8177e951cdb5a0d9670e7a499491d209a8dfbc9c).
* `modifiers` field in `EvMouse`. [Commit](https://github.com/frang75/nappgui_src/commit/68a59be2fd922f0738daeba10fbfb82ed779a693).
* `tag` field in `EvMouse`. [Commit](https://github.com/frang75/nappgui_src/commit/711ea786b8d5dabede70180b57e2414fc8b3533c).
* `listbox_OnDown()`. [Commit](https://github.com/frang75/nappgui_src/commit/711ea786b8d5dabede70180b57e2414fc8b3533c).
* Overlay windows (flyout) [Issue](https://github.com/frang75/nappgui_src/issues/50). [Doc](https://nappgui.com/en/gui/window.html#h4). [Commit](https://github.com/frang75/nappgui_src/commit/ee5c8c8d76b3604151f5d4bbea2e2f405dbfc2ed).
    * `window_overlay()`.
    * `window_control_frame()` and `window_client_to_screen()` [Issue](https://github.com/frang75/nappgui_src/issues/103).
    * `gui_OnIdle()`.
* Dynamic layouts [Issue](https://github.com/frang75/nappgui_src/issues/74). [Doc](https://nappgui.com/en/gui/layout.html#h6).
    * `arrpt_insert_n()`.
    * `cell_empty()`.
    * `layout_panel_replace()` [Issue](https://github.com/frang75/nappgui_src/issues/18).
    * `guicontrol_tag()`.
    * `guicontrol_get_tag()`.
    * `layout_ncols()`.
    * `layout_nrows()`.
    * `layout_insert_col()`.
    * `layout_insert_row()`.
    * `layout_remove_col()`.
    * `layout_remove_row()`.
* `label_size_text()`. [Doc](https://nappgui.com/en/gui/label.html#f5). [Commit](https://github.com/frang75/nappgui_src/commit/e63bd8aaf157ad029e0174fcabc2987393d6885f).
* `window_focus_info`.

### Improved

* DrawBig demo [Doc](https://nappgui.com/en/howto/drawbig.html). [Commit](https://github.com/frang75/nappgui_src/commit/e63bd8aaf157ad029e0174fcabc2987393d6885f).

### Fixed

* macOS Sonoma OSView clipping. [Commit](https://github.com/frang75/nappgui_src/commit/ac038f90e555f8751e4dab1889e0448b47c900ae). [Doc](https://developer.apple.com/documentation/macos-release-notes/appkit-release-notes-for-macos-14#NSView).
* macOS Sonoma NSMenu check obsolete. [Doc](https://developer.apple.com/documentation/macos-release-notes/appkit-release-notes-for-macos-14#Menus). [Issue](https://github.com/frang75/nappgui_src/issues/96). [Commit](https://github.com/frang75/nappgui_src/commit/197c7db6c41b8bde7e01532d4943ee95d4e8ca7e).
* macOS Sonoma OSLabel clipping. [Commit](https://github.com/frang75/nappgui_src/commit/d7930efee0031f8b4269601fe767a695e5e9ff20).
* Windows XP THEME issue. [Commit](https://github.com/frang75/nappgui_src/commit/7f4d10bbd53a26c3e6719c859ab585bb53d6888f).
* `modifiers` field in `EvKey` issue. [Issue](https://github.com/frang75/nappgui_src/issues/95). [Commit](https://github.com/frang75/nappgui_src/commit/68a59be2fd922f0738daeba10fbfb82ed779a693).
* `listbox_color` issue in macOS and GTK+3. [Commit](https://github.com/frang75/nappgui_src/commit/5abe4d13e767ddd1145dfbe5c0cc2b9c0d7c26ab).
* macOS version adapts to Light/Dark/Accents colors at runtime. [Commit](https://github.com/frang75/nappgui_src/commit/46a3e4aa3dd920bd85d89b3387e3b2169b3a0b1e).
* Read/write memory streams `END_BIT` issue. [Commit](https://github.com/frang75/nappgui_src/commit/3a75a1e7bb261fe567f72e487696079ca8eae935).
* Missed unix implementation of `bfile_dir_work()`. [Commit](https://github.com/frang75/nappgui_src/commit/5b92d4fd6ca7ebabea180022de52e101bfb323b5).
* macOS `comwin_open_file()` issue with file types. [Issue](https://github.com/frang75/nappgui_src/issues/107). [Commit](https://github.com/frang75/nappgui_src/commit/32acdb539adeed859986cb6472c34c29a4b0ebd0).
* macOS `ekGUI_CURSOR_SIZENS` bitmap [Commit](https://github.com/frang75/nappgui_src/commit/5b247f67a1ac386ac75f2053c715010d9049f0e4).

### Build system

* Fix Multi-config install issue [Commit](https://github.com/frang75/nappgui_src/commit/5eb39e984bbebed023bc4846dc5e99e318ab89f8).
* Compiler warning in `dlib_proc`. [Commit](https://github.com/frang75/nappgui_src/commit/357960f16e80b9ac72d06c9fb10c9b17fcc4b520).
* GCC Warnings. [Commit](https://github.com/frang75/nappgui_src/commit/16bae6f9c0540e54282a30b1ead3bbf9604b0e7f).
* RaspPI ARM7. [Commit](https://github.com/frang75/nappgui_src/commit/28f86a97d9e82174babea15fb040d645ebf288a2).

## v1.4.0 - Dec 31, 2023 (r4772)

### Added

* String functions: `str_relpath()`, `str_crelpath()` and `str_splits()`.
* `guicontrol` base object [Doc](https://nappgui.com/en/gui/guicontrol.html).
* `layout_control()` and `cell_control()`.
* TableView freeze column [Doc](https://nappgui.com/en/gui/tableview.html#h5). `tableview_column_freeze()`.
* TableView row click event [Doc](https://nappgui.com/en/gui/tableview.html#h6). `tableview_OnRowClick()`, `EvTbRow` and `ekGUI_EVENT_TBL_ROWCLICK`.
* TableView force row height [Doc](https://nappgui.com/en/gui/tableview.html#h7). `tableview_header_height()` and `tableview_row_height()`.
* TableView column-by-column horizontal navigation [Doc](https://nappgui.com/en/gui/tableview.html#h4). `tableview_hkey_scroll()`.
* TableView focus row [Doc](https://nappgui.com/en/gui/tableview.html#h4). `tableview_focus_row()`, `tableview_get_focus_row()`.
* TableView hide scrollbars [Doc](https://nappgui.com/en/gui/tableview.html#h7). `tableview_scroll_visible()`, `view_scroll_visible()` and `view_scroll_size()`.
* TableView multiline headers [Doc](https://nappgui.com/en/gui/tableview.html#h5). `'\n'` in `tableview_header_title()`.
* TextView get full text: `textview_get_text()`.
* TextView select text: `textview_select()`.
* TextView scroll to caret: `textview_scroll_caret()`.
* TextView hide scrollbars: `textview_scroll_visible()`.
* TextView new events: `textview_OnFilter()`, `textview_OnFocus()`.
* TextView clipboard: `textview_copy()`, `textview_cut()`, `textview_paste()`.
* `edit_OnFilter()`: Number of characters inserted or deleted in caret position `EvText::len`.
* `edit_OnChange()`: If event returns `FALSE`, the focus remains in control.
* Edit focus detection: `edit_OnFocus()`.
* Edit select text: `edit_select()`.
* Edit clipboard: `edit_copy()`, `edit_cut()`, `edit_paste()`.
* Keyboard focus result: `gui_focus_t`, `window_focus()`, `window_get_focus()`, `view_OnResignFocus()` and `view_OnAcceptFocus()`.
* Non-cycle tabstops: `window_cycle_tabstop()`.
* Window hotkey support: `window_hotkey()`, `window_clear_hotkeys()` [Doc](https://nappgui.com/en/gui/window.html#h6).
* Window modal avoid hide when close: `ekWINDOW_MODAL_NOHIDE` [Doc](https://nappgui.com/en/gui/window.html#h3).
* View scrollbar event: `view_OnScroll()`, `gui_scroll_t`.
* View new constructor: `view_custom()`.
* View draw overlays: `view_OnOverlay()` [Doc](https://nappgui.com/en/gui/view.html#h3).
* Panel new constructor: `panel_custom()`.
* Button vertical padding: `button_vpadding()` and `button_get_height()`.
* Edit vertical padding: `edit_vpadding()`.
* `blib_getenv()` and `blib_end()`.
* Date functions: `blib_strftime()`, `date_pack()`, `date_ellapsed_seconds()`, `date_format()`, `date_is_valid()`, `date_weekday()`.
* `gui_info_window()`.

### Improved

* Tabstops [Doc](https://nappgui.com/en/gui/layout.html#h6).
* View border change color when focus (focus ring).
* Window keyboard focus [Doc](https://nappgui.com/en/gui/window.html#h4).
* Window default button [Doc](https://nappgui.com/en/gui/window.html#h5).
* `splitview_text()` and `splitview_view()` add a new tabstop parameter.
* Clang-format in all source code.

### Fixed

* Vulnerability in `str_split_trim()`.
* TableView scrollers always visible: [Issue](https://github.com/frang75/nappgui_src/issues/43).
* `edit_color()` issue.
* ListBox scrollbar adjustment.
* Support for full modal application where all windows are modal.
* Removed visible 64x64 artifact when ImageView is used in Windows.
* macOS Panel automatic scrolls when non-visible child control is TAB-focused.
* blib secure functions: `blib_strcpy()`, `blib_strncpy()` and `blib_strcat()`. [Issue](https://github.com/frang75/nappgui_src/issues/61).
* Issue Win32 Alt+Tab navigation with modal windows.
* macOS TableView drawing.
* GTK3 "dummy" control navigation with arrow keys.
* GTK3 lost focus when ALT+TAB navigation.
* GTK3 focus a control when NO tabstops in window.
* GTK3 issue in TextView editable.
* GTK3 get screen resolution.
* window_OnMoved crash on macOS.
* window_OnMoved GTK implementation.

### Removed

* `layout_next_tabstop()` and `layout_previous_tabstop()`.
* `textview_scroll_down()`. Use now `textview_scroll_caret()`.
* `date_DD_MM_YYYY_HH_MM_SS()` and `date_YYYY_MM_DD_HH_MM_SS()`. Use now `date_format()`.
* `date_month_en()` and `date_month_es()`. Use now `date_format()`.

### Build system

* `nrc` compiler builds from source. Deleted binary executables from repo. [Issue](https://github.com/frang75/nappgui_src/issues/44), [Issue](https://github.com/frang75/nappgui_src/issues/22).
* Support for `cmake --install`.
* Support for CMake `find_package()`.
* Support for CMake Ninja generator.
* Support for Clang in Linux.
* Headers in a single directory. [Issue](https://github.com/frang75/nappgui_src/issues/54).
* `NAppProject.cmake` module for help in NAppGUI-based applications.
   * `nap_project_desktop_app()`.
   * `nap_project_command_app()`.
   * `nap_project_library()`.
   * `nap_config_compiler()`.
   * `nap_target_c_standard()`.
   * `nap_target_cxx_standard()`.

## v1.3.1 - Sep 05, 2023 (r4181)

### Added

* Methods to move to next/previous tabstop. [Commit](https://github.com/frang75/nappgui_src/commit/22d30249fa6614f31a994d76d630295a8b334694), [Doc](https://nappgui.com/en/gui/layout.html#h6).
    * `layout_next_tabstop`
    * `layout_previous_tabstop`
    * `window_next_tabstop`
    * `window_previous_tabstop`

### Fixed

* `.def` files compatibles with BUSY build system. [Issue](https://github.com/frang75/nappgui_src/issues/4#issuecomment-1376431375), [Commit](https://github.com/frang75/nappgui_src/commit/34b3f2551c4b884e65270b0f35f50240a26e0935).
* Bug in `SplitView` with nested panels. [Issue](https://github.com/frang75/nappgui_src/issues/33), [Commit](https://github.com/frang75/nappgui_src/commit/dfaf85f801890e0dcd62f31836d87e3fc2267ff6).
* Bug in `LexicalScanner`. [Commit](https://github.com/frang75/nappgui_src/commit/57bf61c42f8617e5d73117c9d51b8890248f8921).
* macOS issue in empty-text checkboxes. [Commit](https://github.com/frang75/nappgui_src/commit/57bf61c42f8617e5d73117c9d51b8890248f8921).
* Json issues and improved documentation. [Commit](https://github.com/frang75/nappgui_src/commit/57bf61c42f8617e5d73117c9d51b8890248f8921), [Doc](https://nappgui.com/en/inet/json.html).
* macOS Lion edit focus issue. [Commit](https://github.com/frang75/nappgui_src/commit/22d30249fa6614f31a994d76d630295a8b334694).


## v1.3.0 - Dec 26, 2022 (r4146)

### Added

* New header `<blib.h>` [Doc](https://nappgui.com/en/sewer/blib.html).
* New header `<dlib.h>` [Doc](https://nappgui.com/en/osbs/dlib.html).
* Support for TableView [Doc](https://nappgui.com/en/gui/tableview.html).
* Compiler definitions for static/dynamic functions export/import
  * `_sewer_api`
  * `_osbs_api`
  * `_core_api`
  * `_geom2d_api`
  * `_draw2d_api`
  * `_osgui_api`
  * `_gui_api`
  * `_osapp_api`
  * `_inet_api`

### Improved

* Full review of ANSI C90
* New .pdf ebook edition

### Build system

* Support for generate dynamic libraries `dynamicLib()`. [Doc](https://nappgui.com/en/guide/build.html#h2).
* Possibility to choose the standard C/C++ per project. [Doc](https://nappgui.com/en/guide/newprj.html#h4).
* `staticLib()` adds libName parameter.
* `commandLib()` adds appName parameter.
* `dynamicLib()` new build command.
* Support for multiple targets from same source folder.
* `generateSolution() `should be added at the end of src/CMakeLists.txt


## v1.2.4 - Dec 26, 2022 (r3888)

### Fixed

* Fix Right-Shift tabstop in Windows.
* GTK+3 issue in Wayland (forced X11).


## v1.2.3 - Jun 5, 2022 (r3778)

### Added
* Linux ARM64 build support.
* Build support for "AppleClang" compiler ID.

### Fixed

* TextView rich text working on macOS
* Solved compile warnings with macOS SDK 12.0


## v1.2.2 - Apr 30, 2022 (r3744)

### Added

* GCC11 Ubuntu 22.04 support.

### Fixed

* GTK3 issues in older versions (Ubuntu 12, 14, 16).
* Proc deadlock in Win32 `bproc_read_close`.


## v1.2.1 - Apr 9, 2022 (r3657)

### Added

* `buffer_const()`
* `buffer_with_data()`
* `heap_leaks()`
* `heap_stats()`
* `bfile_pos()`
* `bfile_seek()`
* `layout_dbind_update()`
* `evbind_modify()`
* `evbind_object()`
* `r2d_centerf()`
* VisualStudio 2022 support.
* Xcode 13 macOS Monterey support.

### Improved

* Containers 'const' treatment (Arrays, Sets).
* Linux Gtk3/Curl/Pthread package dependencies management.
* GUI data binding implementation and docs: [Doc](https://nappgui.com/en/gui/gbind.html).

### Fixed

* GIF animations in GTK/Linux.


## v1.2.0 - Oct 16, 2021 (r3382)

* First Open Source release.

