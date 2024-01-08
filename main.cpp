#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <cmath>
#include <string.h>

typedef struct
{
    double x;
    double y;
    const gchar *content;
} Point;

typedef struct
{
    GList *points;
    int type;
} DrawNode;

GList *drawNodeList = NULL;

int tag = 0;

gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    GdkWindow *window = gtk_widget_get_window(widget);
    cairo_t *cr = gdk_cairo_create(window);

    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

    int width, height;
    gtk_widget_get_size_request(widget, &width, &height);

    cairo_set_line_width(cr, 2);

    for (GList *iter = drawNodeList; iter != NULL; iter = g_list_next(iter))
    {
        DrawNode *node = (DrawNode *)iter->data;
        if (node->type == 1)
        {
            GList *points = (GList *)node->points;

            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_move_to(cr, ((Point *)points->data)->x, ((Point *)points->data)->y);

            for (GList *point_iter = points->next; point_iter != NULL; point_iter = g_list_next(point_iter))
            {
                cairo_line_to(cr, ((Point *)point_iter->data)->x, ((Point *)point_iter->data)->y);
            }

            cairo_stroke(cr);
        }
        else if (node->type == 2)
        {
            GList *points = (GList *)node->points;

            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_move_to(cr, ((Point *)points->data)->x, ((Point *)points->data)->y);

            Point *first_point = (Point *)points->data;
            gint x1 = first_point->x;
            gint y1 = first_point->y;
            Point *last_point = (Point *)g_list_last(points)->data;
            gint x2 = last_point->x;
            gint y2 = last_point->y;

            gint rect_width = x2 - x1;
            gint rect_height = y2 - y1;
            cairo_rectangle(cr, x1, y1, rect_width, rect_height);

            cairo_stroke(cr);
        }
        else if (node->type == 3)
        {
            GList *points = (GList *)node->points;

            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_move_to(cr, ((Point *)points->data)->x, ((Point *)points->data)->y);

            Point *first_point = (Point *)points->data;
            gint x1 = first_point->x;
            gint y1 = first_point->y;
            Point *last_point = (Point *)g_list_last(points)->data;
            gint x2 = last_point->x;
            gint y2 = last_point->y;

            double width = fabs(x2 - x1);
            double height = fabs(y2 - y1);
            double center_x = (x1 + x2) / 2;
            double center_y = (y1 + y2) / 2;

            double radius_x = width / 2;
            double radius_y = height / 2;

            cairo_save(cr);
            cairo_translate(cr, center_x, center_y);
            cairo_scale(cr, radius_x, radius_y);
            cairo_new_path(cr);
            cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
            cairo_restore(cr);

            cairo_stroke(cr);
        }
        else if (node->type == 4)
        {
            GList *points = (GList *)node->points;
            Point *point = (Point *)points->data;
            cairo_set_source_rgb(cr, 1, 1, 1);

            cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size(cr, 20);

            cairo_move_to(cr, point->x, point->y);
            cairo_show_text(cr, point->content);

            cairo_stroke(cr);
        }
        else if (node->type == 5)
        {
            GList *points = (GList *)node->points;

            cairo_set_source_rgba(cr, 0, 0, 0, 0);
            cairo_move_to(cr, ((Point *)points->data)->x, ((Point *)points->data)->y);

            for (GList *point_iter = points->next; point_iter != NULL; point_iter = g_list_next(point_iter))
            {
                cairo_rectangle(cr, ((Point *)point_iter->data)->x - 10 / 2, ((Point *)point_iter->data)->y - 10 / 2, 10, 10);
                cairo_fill(cr);
            }

            cairo_stroke(cr);
        }
    }

    cairo_destroy(cr);

    return FALSE;
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    gchar *key = gdk_keyval_name(event->keyval);
    if (strcmp(key, "1") == 0)
    {
        g_print("now line !!!\n");
        tag = 1; //line
    }
    else if (strcmp(key, "2") == 0)
    {
        g_print("now rect !!!\n");
        tag = 2; //rect
    }
    else if (strcmp(key, "3") == 0)
    {
        g_print("now round !!!\n");
        tag = 3; //round
    }
    else if (strcmp(key, "4") == 0)
    {
        g_print("now text !!!\n");
        tag = 4; //text
    }
    else if (strcmp(key, "5") == 0)
    {
        g_print("now eraser !!!\n");
        tag = 5;
    }
    else if (strcmp(key, "6") == 0)
    {
        g_print("now revoke !!!\n");
        tag = 6;
        drawNodeList = g_list_delete_link(drawNodeList, g_list_last(drawNodeList));
        gtk_widget_queue_draw(widget);
    }
    else
    {
        if (tag == 4)
        {
            DrawNode *node = (DrawNode *)g_list_last(drawNodeList)->data;
            GList *points = (GList *)node->points;
            Point *point = (Point *)points->data;
            gchar *content = g_strdup(point->content);
            gchar *temp = g_strconcat(content, key, NULL);
            point->content = temp;

            gtk_widget_queue_draw(widget);
        }
    }

    return TRUE;
}

gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if (event->button == 1)
    {
        GList *points = NULL;

        Point *point = g_new(Point, 1);
        point->x = event->x;
        point->y = event->y;
        point->content = "";
        points = g_list_append(points, point);

        DrawNode *drawNode = g_new(DrawNode, 1);
        drawNode->points = points;
        drawNode->type = tag;

        drawNodeList = g_list_append(drawNodeList, drawNode);
        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}

gboolean on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
    if (event->state & GDK_BUTTON1_MASK)
    {
        DrawNode *node = (DrawNode *)g_list_last(drawNodeList)->data;
        GList *points = (GList *)node->points;

        Point *point = g_new(Point, 1);
        point->x = event->x;
        point->y = event->y;
        point->content = "";

        points = g_list_append(points, point);

        g_list_last(drawNodeList)->data = node;

        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_title(GTK_WINDOW(window), "Transparent Window");

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(drawing_area, "expose-event", G_CALLBACK(on_expose_event), NULL);

    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(drawing_area, "motion-notify-event", G_CALLBACK(on_motion_notify), NULL);

    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    GdkScreen *screen = gtk_widget_get_screen(window);
    GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);
    if (!colormap)
    {
        colormap = gdk_screen_get_rgb_colormap(screen);
    }
    gtk_widget_set_colormap(window, colormap);

    gtk_widget_show_all(window);

    gtk_main();

    g_list_free_full(drawNodeList, (GDestroyNotify)g_list_free_full);

}