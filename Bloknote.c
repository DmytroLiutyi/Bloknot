#include <gtk/gtk.h>

static void on_open_file(GtkWidget *widget, gpointer text_view);
static void on_save_file(GtkWidget *widget, gpointer data);
static void on_new_file(GtkWidget *widget, gpointer data);
static void on_find_text(GtkWidget *widget, gpointer text_view);
static void on_replace_text(GtkWidget *widget, gpointer text_view);
static void on_color_selected(GtkWidget *widget, gpointer data);
static void on_find_button_clicked(GtkWidget *find_button, gpointer data);
static void on_find_dialog_close(GtkWidget *widget, gint response_id, gpointer data);

typedef struct {
    GtkWidget *entry;
    GtkWidget *text_view;
} FindData;

typedef struct {
    int start;
    int end;
    GdkRGBA *color;
} ColorRegion;

typedef struct {
    GtkWidget *text_view;
    ColorRegion *color_regions[200];
} ColorSelectionData;

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *menubar;
    GtkWidget *file_menu;
    GtkWidget *edit_menu;
    GtkWidget *file_mi;
    GtkWidget *edit_mi;
    GtkWidget *new_mi;
    GtkWidget *open_mi;
    GtkWidget *save_mi;
    GtkWidget *exit_mi;
    GtkWidget *find_mi;
    GtkWidget *replace_mi;
    GtkWidget *color_mi;
    GtkWidget *scroll;
    GtkWidget *text_view;
    GtkTextBuffer *buffer;
    GtkWidget *dialog; 
    ColorRegion *color_regions[200] = {NULL};
    ColorSelectionData *color_selection_data = g_malloc(sizeof(ColorSelectionData) + 200 * sizeof(ColorRegion *));

    for (int i = 0; i < 200; i++) {
    color_selection_data->color_regions[i] = color_regions[i];
    }
    
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Блокнот");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    menubar = gtk_menu_bar_new();
    file_menu = gtk_menu_new();
    edit_menu = gtk_menu_new();

    file_mi = gtk_menu_item_new_with_label("File");
    new_mi = gtk_menu_item_new_with_label("New");
    open_mi = gtk_menu_item_new_with_label("Open");
    save_mi = gtk_menu_item_new_with_label("Save");
    exit_mi = gtk_menu_item_new_with_label("Exit");

    edit_mi = gtk_menu_item_new_with_label("Edit");
    find_mi = gtk_menu_item_new_with_label("Find");
    replace_mi = gtk_menu_item_new_with_label("Replace");
    color_mi = gtk_menu_item_new_with_label("Color");

    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), exit_mi);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_mi), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_mi);

    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), find_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), replace_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), color_mi);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_mi), edit_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit_mi);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    text_view = gtk_text_view_new();
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_create_tag(buffer, "highlight", "background", "yellow", NULL);

    color_selection_data->text_view = text_view;

    g_signal_connect(new_mi, "activate", G_CALLBACK(on_new_file), color_selection_data);
    g_signal_connect(open_mi, "activate", G_CALLBACK(on_open_file), color_selection_data);
    g_signal_connect(save_mi, "activate", G_CALLBACK(on_save_file), color_selection_data);
    g_signal_connect(exit_mi, "activate", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(find_mi, "activate", G_CALLBACK(on_find_text), text_view);
    g_signal_connect(replace_mi, "activate", G_CALLBACK(on_replace_text), text_view);
    g_signal_connect(color_mi, "activate", G_CALLBACK(on_color_selected), color_selection_data);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

static void on_new_file(GtkWidget *widget, gpointer data) {
    ColorSelectionData *color_selection_data = (ColorSelectionData *)data;
    GtkWidget *text_view = color_selection_data->text_view;

    for(int i = 0; i < 200; i++) {
        color_selection_data->color_regions[i] = NULL;
    }

    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, "", -1);
}

void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

char *convertColorRegionsToString(ColorRegion *regions[]) {
    int maxStringLength = 200 * 2000; 
    char *result = (char *)malloc(maxStringLength * sizeof(char));

    result[0] = '\0';
    
    for (int i = 0; i < 200; i++) {
        ColorRegion *region = regions[i];
        if(i != 0 && region != NULL) strcat(result, ";");
        if (region == NULL) break;

        sprintf(result + strlen(result), "%d %d ", region->start, region->end);
        sprintf(result + strlen(result), "%s", gdk_rgba_to_string(region->color));
    }
    return result;
}

char* concatWithNewline(const char* str1, const char* str2) {
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    size_t totalLen = len1 + 1 + len2 + 1;

    char* result = (char*)malloc(totalLen * sizeof(char));
    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    strcpy(result, str1);

    result[len1] = '\n';

    strcpy(result + len1 + 1, str2);

    result[totalLen - 1] = '\0';

    return result;
}

static void on_color_open(ColorSelectionData* color_selection_data) {
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    GtkTextTag *tag;

    GtkWidget *text_view = color_selection_data->text_view;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    for (int i = 0; i < 200; i++){
        ColorRegion* color_region = color_selection_data->color_regions[i]; 
        if (color_region == NULL) break;   

        //g_print("%f %f %f %f", color_region->color->red, color_region->color->green, color_region->color->blue, color_region->color->alpha);

        char tag_name[] = { [41] = '\1' }; 
        rand_str(tag_name, sizeof tag_name - 1);

        gtk_text_buffer_get_iter_at_offset(buffer, &start, color_region->start);
        gtk_text_buffer_get_iter_at_offset(buffer, &end, color_region->end);

        tag = gtk_text_buffer_create_tag(buffer, tag_name, "foreground-rgba", color_region->color, NULL);

        if (!gtk_text_iter_equal(&start, &end)) {
            gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        }
    }
}


void rgbStringToArray(const char *rgbString, int rgbArray[3]) {
    int r, g, b;
    if (sscanf(rgbString, "rgb(%d,%d,%d)", &r, &g, &b) == 3) {
        rgbArray[0] = r;
        rgbArray[1] = g;
        rgbArray[2] = b;
    } else {
        printf("Invalid RGB string format: %s\n", rgbString);
    }
}

ColorRegion** parseColorRegions(char* input) {
    static const int max_regions = 200;
    ColorRegion **regions = g_malloc(sizeof(ColorRegion)*max_regions);

    char* token = strtok(input, ";");
    int index = 0;

    while (token != NULL && index < max_regions) {
        ColorRegion *region = g_malloc(sizeof(ColorRegion));
        GdkRGBA *color = g_malloc(sizeof(GdkRGBA));
        char color_str[64];
        int rgbArray[3] ={0};
        
        sscanf(token, "%d %d %63s", &region->start, &region->end, color_str); 
        gdk_rgba_parse(color, color_str);
        region->color = color;
        regions[index++] = region;
        token = strtok(NULL, ";");
    }
    for (int i = index; i < max_regions; ++i){
        regions[i] = NULL;
    }
    
    return regions;
}

void split_lines(const char *input, char **first_line, char **subsequent_lines) {
    const char *newline_pos = strchr(input, '\n');
    if (newline_pos == NULL) {
        *first_line = strdup(input);
        *subsequent_lines = NULL;
        return;
    }

    size_t first_line_length = newline_pos - input;


    *first_line = (char *)malloc(first_line_length + 1);

    strncpy(*first_line, input, first_line_length);
    (*first_line)[first_line_length] = '\0'; 


    *subsequent_lines = strdup(newline_pos + 1);
}

static void on_open_file(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    
    ColorSelectionData *color_selection_data = (ColorSelectionData *)data;
    GtkWidget *text_view = color_selection_data->text_view;

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Open", GTK_RESPONSE_ACCEPT,
                                         NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        char *content;
        gsize length;
        GError *error = NULL;

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (g_file_get_contents(filename, &content, &length, &error)) {

            char* selection_data;
            char* text;
            split_lines(content, &selection_data, &text);
            ColorRegion **color_regions = parseColorRegions(selection_data);
            
            for (int i = 0; i < 200; i++) {
                color_selection_data->color_regions[i] = color_regions[i];

            }

            buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(buffer, text, strlen(text));
            on_color_open(color_selection_data);
            g_free(content);
        } else {
            g_error("Error opening file: %s\n", error->message);
            g_clear_error(&error);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_save_file(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    ColorSelectionData *color_selection_data = (ColorSelectionData *)data;
    GtkWidget *text_view = color_selection_data->text_view;
    
    dialog = gtk_file_chooser_dialog_new("Save File",
                                         GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Save", GTK_RESPONSE_ACCEPT,
                                         NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {

        char *filename;
        char *content;
        GError *error = NULL;

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        content = concatWithNewline(convertColorRegionsToString(color_selection_data->color_regions),content);
        
        g_file_set_contents(filename, content, -1, &error);

        g_free(content);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_find_button_clicked(GtkWidget *find_button, gpointer user_data) {
    FindData *data = (FindData *)user_data;
    GtkWidget *entry = data->entry;
    GtkWidget *text_view = data->text_view;
    
    const gchar *text_to_find = gtk_entry_get_text(GTK_ENTRY(entry));
    GtkTextIter start, match_start, match_end;
    gboolean found;

    if (!GTK_IS_TEXT_VIEW(text_view)) {
        return;
    }
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    GtkTextTag *highlight_tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer), "highlight");
    gtk_text_buffer_get_start_iter(buffer, &start);

    // Remove old highlight tags
    //gtk_text_buffer_remove_tag_by_name(buffer, "highlight", &start, &start);

    while (gtk_text_iter_forward_search(&start, text_to_find, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL)) {
        gtk_text_buffer_apply_tag(buffer, highlight_tag, &match_start, &match_end);
        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view), &match_start, 0.0, TRUE, 0.5, 0.5);
        start = match_end;
    }
}

static void on_find_dialog_close(GtkWidget *widget, gint response_id, gpointer data) {
    if (response_id != GTK_RESPONSE_CLOSE)
    {
        return;
    }
    FindData *find_data = (FindData *)data;
    GtkWidget *text_view = find_data->text_view;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));


    GtkTextIter start;
    gtk_text_buffer_get_start_iter(buffer, &start);
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_remove_tag_by_name(buffer, "highlight", &start, &end);
    g_print("%s",gtk_text_iter_get_text(&start, &end));
    g_print("ssss");
    // Debug message to ensure function is called properly
    g_print("on_find_dialog_close: Highlight tags removed.\n");
}


static void on_find_text(GtkWidget *widget, gpointer text_view) {
    GtkWidget *dialog, *content_area, *entry, *find_button;
    FindData *data = g_malloc(sizeof(FindData));

    dialog = gtk_dialog_new_with_buttons("Find Text",
                                         GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_Close", GTK_RESPONSE_CLOSE,
                                         NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    find_button = gtk_button_new_with_label("Find");

    gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), find_button, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    data->entry = entry;
    data->text_view = GTK_WIDGET(text_view);

    g_signal_connect(find_button, "clicked", G_CALLBACK(on_find_button_clicked), data);
    g_signal_connect(dialog, "response", G_CALLBACK(on_find_dialog_close), data);

    gtk_dialog_run(GTK_DIALOG(dialog));
    g_free(data);
    gtk_widget_destroy(dialog);
}

static void on_replace_text(GtkWidget *widget, gpointer text_view) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *find_entry;
    GtkWidget *replace_entry;
    GtkWidget *grid;
    GtkTextBuffer *buffer;
    GtkTextIter start, match_start, match_end;
    gboolean found;

    dialog = gtk_dialog_new_with_buttons("Replace Text",
                                         GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_Replace", GTK_RESPONSE_ACCEPT,
                                         "_Close", GTK_RESPONSE_CLOSE,
                                         NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Find:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Replace:"), 0, 1, 1, 1);
    find_entry = gtk_entry_new();
    replace_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), find_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), replace_entry, 1, 1, 1, 1);
    gtk_box_pack_start(GTK_BOX(content_area), grid, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        const gchar *text_to_find = gtk_entry_get_text(GTK_ENTRY(find_entry));
        const gchar *text_to_replace = gtk_entry_get_text(GTK_ENTRY(replace_entry));
        gtk_text_buffer_get_start_iter(buffer, &start);

        found = gtk_text_iter_forward_search(&start, text_to_find, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL);
        if (found) {
            gtk_text_buffer_delete(buffer, &match_start, &match_end);
            gtk_text_buffer_insert(buffer, &match_start, text_to_replace, -1);
            gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view), &match_start, 0.0, TRUE, 0.5, 0.5);
        }
    }
    gtk_widget_destroy(dialog);
}


static void on_color_selected(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    GtkTextTag *tag;
    GdkRGBA color;
    ColorSelectionData *color_selection_data = (ColorSelectionData *)data;
    GtkWidget *text_view = color_selection_data->text_view;

    dialog = gtk_color_chooser_dialog_new("Choose Color",
                                          GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(text_view))));
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);

        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

        char tag_name[] = { [41] = '\1' };
        rand_str(tag_name, sizeof tag_name - 1);

        tag = gtk_text_buffer_create_tag(buffer, tag_name, "foreground-rgba", &color, NULL);

        gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
        if (!gtk_text_iter_equal(&start, &end)) {
            gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
            ColorRegion *color_region = g_malloc(sizeof(FindData));
            color_region->start = gtk_text_iter_get_offset(&start);
            color_region->end = gtk_text_iter_get_offset(&end);
            GdkRGBA *temp_color = gdk_rgba_copy(&color);
            color_region->color = temp_color;
            
            for (int i = 0; i<200; i++)
            {
                if (color_selection_data->color_regions[i] == NULL){
                    color_selection_data->color_regions[i] = color_region;
                    break;
                }
            }
            

        }
        
        
        // Ensure the tag is correctly applied
        //g_object_unref(tag);
    }
    gtk_widget_destroy(dialog);
}