/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bricks.c
 *
 */

/* Simplistic Breakout-like game */

#include "nappgui.h"

#define NUM_BRICKS 40

typedef struct _brick_t Brick;
typedef struct _app_t App;

struct _brick_t
{
    real32_t x;
    real32_t y;
    uint8_t color;
    bool_t is_visible;
};

struct _app_t
{
    bool_t is_running;
    Brick bricks[NUM_BRICKS];
    color_t color[4];
    real32_t brick_width;
    real32_t player_pos;
    real32_t ball_x;
    real32_t ball_y;
    V2Df ball_dir;
    real32_t ball_speed;
    Cell *button;
    Slider *slider;
    View *view;
    Window *window;
};

/*---------------------------------------------------------------------------*/

static const real32_t i_BALL_RADIUS = .03f;
static const real32_t i_BRICK_HEIGHT = .03f;
static const real32_t i_BRICK_SEPARATION = .005f;
static const uint32_t i_BRICKS_PER_ROW = 10;
static const uint32_t i_NUM_ROWS = 4;

/*---------------------------------------------------------------------------*/

static void i_OnDraw(App *app, Event *e)
{    
    const EvDraw *params = event_params(e, EvDraw);
    uint32_t i = 0;

    draw_clear(params->ctx, color_rgb(102, 153, 26));
    draw_line_color(params->ctx, kCOLOR_BLACK);

    for (i = 0; i < NUM_BRICKS; ++i)
    {
        if (app->bricks[i].is_visible == TRUE)
        {
            real32_t x = app->bricks[i].x * params->width;
            real32_t y = app->bricks[i].y * params->height;
            real32_t width = app->brick_width * params->width;
            real32_t height = i_BRICK_HEIGHT * params->height;
            draw_fill_color(params->ctx, app->color[app->bricks[i].color]);
            draw_rect(params->ctx, ekFILLSK, x, y, width, height);
        }
    }
    
    {
        real32_t x = (app->player_pos - app->brick_width) * params->width;
        real32_t y = (1 - i_BRICK_HEIGHT - i_BRICK_SEPARATION) * params->height;
        real32_t width = 2 * app->brick_width * params->width;
        real32_t height = i_BRICK_HEIGHT * params->height;
        draw_fill_color(params->ctx, kCOLOR_BLACK);
        draw_rect(params->ctx, ekFILL, x, y, width, height);
    }

    {
        real32_t x = app->ball_x * params->width;
        real32_t y = app->ball_y * params->height;
        real32_t rad = i_BALL_RADIUS * params->width;
        draw_fill_color(params->ctx, kCOLOR_WHITE);
        draw_circle(params->ctx, ekFILL, x, y, rad);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnSlider(App *app, Event *e)
{
    const EvSlider *params = event_params(e, EvSlider);
    app->player_pos = params->pos;
}

/*---------------------------------------------------------------------------*/

static void i_OnStart(App *app, Event *e)
{
    unref(e);
    app->is_running = TRUE;
    cell_enabled(app->button, FALSE);
}

/*---------------------------------------------------------------------------*/

 static Panel *i_panel(App *app)
 {
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 4);
    View *view = view_create();
    Slider *slider = slider_create();
    Label *label = label_create();
    Button *button = button_push();
    view_size(view, s2df(258, 344));
    view_OnDraw(view, listener(app, i_OnDraw, App)); 
    slider_OnMoved(slider, listener(app, i_OnSlider, App));
    label_text(label, "Use the slider!");
    button_text(button, "Start");
    button_OnClick(button, listener(app, i_OnStart, App));    
    layout_view(layout, view, 0, 0);
    layout_slider(layout, slider, 0, 1);
    layout_label(layout, label, 0, 2);
    layout_button(layout, button, 0, 3);
    layout_vexpand(layout, 0);
    layout_vmargin(layout, 0, 10);
    layout_vmargin(layout, 2, 10);
    layout_margin(layout, 10);
    panel_layout(panel, layout);
    app->view = view;
    app->slider = slider;
    app->button = layout_cell(layout, 0, 3);
    return panel;
}

/*---------------------------------------------------------------------------*/

static void i_init_game(App *app)
{
    real32_t hoffset;
    Brick *brick = NULL;
    uint32_t j, i;
    
    app->color[0] = color_rgb(255, 0, 0);
    app->color[1] = color_rgb(0, 255, 0);
    app->color[2] = color_rgb(0, 0, 255);
    app->color[3] = color_rgb(0, 255, 255);

    hoffset = i_BRICK_SEPARATION;
    brick = app->bricks;

    app->is_running = FALSE;
    app->brick_width = (1 - ((real32_t)i_BRICKS_PER_ROW + 1) * i_BRICK_SEPARATION) / (real32_t)i_BRICKS_PER_ROW;

    for (j = 0; j < i_NUM_ROWS; ++j)
    {
        real32_t woffset = i_BRICK_SEPARATION;
        
        for (i = 0; i < i_BRICKS_PER_ROW; ++i)
        {
            brick->x = woffset;
            brick->y = hoffset;
            brick->is_visible = TRUE;
            brick->color = (uint8_t)j;
            woffset += app->brick_width + i_BRICK_SEPARATION;
            brick++;
        }
        
        hoffset += i_BRICK_HEIGHT + i_BRICK_SEPARATION;
    }
    
    app->player_pos = slider_get_value(app->slider);
    app->ball_x = .5f;
    app->ball_y = .5f;
    app->ball_dir.x = .3f;
    app->ball_dir.y = -.1f;
    app->ball_speed = .6f;
    v2d_normf(&app->ball_dir);
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    osapp_finish();
    unref(app);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = heap_new0(App);
    Panel *panel = i_panel(app);
    app->window = window_create(ekWNSRES);
    window_panel(app->window, panel);
    window_origin(app->window, v2df(200, 200));
    window_title(app->window, "Bricks - A 2D Game");
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    i_init_game(app);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    window_destroy(&(*app)->window);
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

static bool_t i_collision(Brick *brick, real32_t brick_width, real32_t ball_x, real32_t ball_y)
{
    if (ball_x + i_BALL_RADIUS < brick->x)
        return FALSE;
    if (ball_x - i_BALL_RADIUS > brick->x + brick_width)
        return FALSE;
    if (ball_y + i_BALL_RADIUS < brick->y)
        return FALSE;
    if (ball_y - i_BALL_RADIUS > brick->y + i_BRICK_HEIGHT)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_update(App *app, const real64_t prtime, const real64_t ctime)
{
    if (app->is_running == TRUE)
    {
        real32_t step = (real32_t)(ctime - prtime);
        bool_t collide;
        uint32_t i;
    
        // Update ball position
        app->ball_x += step * app->ball_speed * app->ball_dir.x;
        app->ball_y += step * app->ball_speed * app->ball_dir.y;
    
        // Collision with limits
        if (app->ball_x + i_BALL_RADIUS >= 1.f && app->ball_dir.x >= 0.f)
            app->ball_dir.x = - app->ball_dir.x;
    
        if (app->ball_x - i_BALL_RADIUS <= 0.f && app->ball_dir.x <= 0.f)
            app->ball_dir.x = - app->ball_dir.x;

        if (app->ball_y - i_BALL_RADIUS <= 0.f && app->ball_dir.y <= 0.f)
            app->ball_dir.y = - app->ball_dir.y;
    
        // Collision with bricks
        collide = FALSE;
        for (i = 0; i < NUM_BRICKS; ++i)
        {
            if (app->bricks[i].is_visible == TRUE)
            {
                if (i_collision(&app->bricks[i], app->brick_width, app->ball_x, app->ball_y) == TRUE)
                {
                    app->bricks[i].is_visible = FALSE;
                    if (collide == FALSE)
                    {
                        real32_t brick_x = app->bricks[i].x + .5f * app->brick_width;
                        app->ball_dir.x = 5.f * (app->ball_x - brick_x);
                        app->ball_dir.y = - app->ball_dir.y;
                        v2d_normf(&app->ball_dir);
                        collide = TRUE;
                    }
                }
            }
        }

        // Collision with player
        {
            Brick player;
            player.x = app->player_pos - app->brick_width;
            player.y = 1.f - i_BRICK_HEIGHT - i_BRICK_SEPARATION;
            if (i_collision(&player, 2.f * app->brick_width, app->ball_x, app->ball_y) == TRUE)
            {
                app->ball_dir.x = 5.f * (app->ball_x - app->player_pos);
                app->ball_dir.y = - app->ball_dir.y;
                v2d_normf(&app->ball_dir);
            }
        }

        // Game Over
        if (app->ball_y + i_BALL_RADIUS >= 1.f)
        {
            i_init_game(app);
            cell_enabled(app->button, TRUE);
        }
    }

    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain_sync(.04, i_create, i_destroy, i_update, "", App)
