#include <gtk/gtk.h>

typedef struct {
    GtkEntry *entry;
    GtkListBox *listbox;
    GtkWidget *dialog;
    GtkListBoxRow *row_to_delete;
} TaskData;

// If "x" button is pressed on dialog window, then destroy it
static void on_dialog_destroy(GtkWidget *widget, gpointer user_data) {
    TaskData *local_data = (TaskData *)user_data;
    local_data->dialog = NULL;
}

static void complete_task(GtkCheckButton *checkbox, gpointer user_data) {
    TaskData *local_data = (TaskData *)user_data;

    if (gtk_check_button_get_active(checkbox)) {
        GtkWidget *row = gtk_widget_get_ancestor(GTK_WIDGET(checkbox), GTK_TYPE_LIST_BOX_ROW);

        if (row)
            gtk_list_box_remove(local_data->listbox, row);
    }
}

static gboolean search_filter(GtkListBoxRow *row, gpointer user_data) {
    TaskData *local_data = (TaskData *)user_data;

    if (!row || !local_data || !local_data->entry)
        return TRUE;

    const char *search_text = gtk_editable_get_text(GTK_EDITABLE(local_data->entry));
    if (!search_text || strlen(search_text) == 0)
        return TRUE;

    GtkWidget *box_row = gtk_list_box_row_get_child(row);
    if (!box_row || !GTK_IS_BOX(box_row))
        return TRUE;

    GtkWidget *box_row_task = gtk_widget_get_first_child(box_row);
    if (!box_row_task || !GTK_IS_BOX(box_row_task))
        return TRUE;

    GtkWidget *checkbox = gtk_widget_get_first_child(box_row_task);
    if (!checkbox)
        return TRUE;

    GtkWidget *label = gtk_widget_get_next_sibling(checkbox);
    if (!label || !GTK_IS_LABEL(label))
        return TRUE;

    const char *task_text = gtk_label_get_text(GTK_LABEL(label));
    if (!task_text)
        return FALSE;

    return g_strrstr(task_text, search_text) != NULL;
}

static void on_search_clicked(GtkButton *btn, gpointer user_data) {
    TaskData *local_data = (TaskData *)user_data;
    gtk_list_box_invalidate_filter(local_data->listbox);
}

static void search_dialog(GtkButton *btn, gpointer user_data) {
    TaskData *local_data = (TaskData *)user_data;

    if (local_data->dialog != NULL) {
        gtk_window_present(GTK_WINDOW(local_data->dialog));
        return;
    }

    local_data->dialog = gtk_window_new();
    g_signal_connect(local_data->dialog, "destroy", G_CALLBACK(on_dialog_destroy), local_data);

    gtk_window_set_title(GTK_WINDOW(local_data->dialog), "Search");
    gtk_window_set_default_size(GTK_WINDOW(local_data->dialog), 200, 150);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(local_data->dialog), box);

    local_data->entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(GTK_ENTRY(local_data->entry), "Enter the text find");

    gtk_box_append(GTK_BOX(box), GTK_WIDGET(local_data->entry));

    GtkWidget *search_listbox = gtk_list_box_new();

    /// **************************************** ///

    GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(local_data->listbox));

    if (GTK_IS_LIST_BOX_ROW(child))
        g_print("Found a row child!\n");

    GtkWidget *child2 = gtk_list_box_row_get_child(GTK_LIST_BOX_ROW(child));

    if (GTK_IS_BOX(child2))
        g_print("Found a box child2!\n");

    GtkWidget *child3 = gtk_widget_get_first_child(child2);

    if (GTK_IS_BOX(child3))
        g_print("Found a box child3!\n");

    GtkWidget *child4 = gtk_widget_get_first_child(child3);

    if (GTK_IS_CHECK_BUTTON(child4))
        g_print("Found a checkbox child4!\n");

    GtkWidget *child5 = gtk_widget_get_next_sibling(child4);

    if (GTK_IS_LABEL(child5))
        g_print("Found a label child5!\n");

    /// **************************************** ///

    gtk_box_append(GTK_BOX(box), GTK_WIDGET(search_listbox));

    gtk_window_present(GTK_WINDOW(local_data->dialog));
}

static void on_menu_button_active(GObject *object, GParamSpec *pspec, gpointer user_data) {
    GtkMenuButton *menuBtn = GTK_MENU_BUTTON(object);

    if (gtk_menu_button_get_active(menuBtn)) {
        GtkListBoxRow *row = GTK_LIST_BOX_ROW(user_data);
        GtkWidget *listbox = gtk_widget_get_parent(GTK_WIDGET(row));

        if (GTK_IS_LIST_BOX(listbox)) {
            gtk_list_box_select_row(GTK_LIST_BOX(listbox), row);
            gtk_widget_grab_focus(GTK_WIDGET(row));
        }
    }
}

static void delete_task_confirmed(GtkButton *btn, gpointer user_data) {
    TaskData *local_data = (TaskData *)user_data;

    if (local_data->row_to_delete != NULL)
        gtk_list_box_remove(local_data->listbox, GTK_WIDGET(local_data->row_to_delete));

    if (local_data->dialog) {
        gtk_window_destroy(GTK_WINDOW(local_data->dialog));
        local_data->dialog = NULL;
        local_data->row_to_delete = NULL;
    }
}

static void delete_task_cancelled(GtkButton *btn, gpointer user_data) {
    TaskData *local_data = (TaskData *)user_data;

    if (local_data->dialog) {
        gtk_window_destroy(GTK_WINDOW(local_data->dialog));
        local_data->dialog = NULL;
        local_data->row_to_delete = NULL;
    }
}

static void delete_task_dialog(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    TaskData *local_data = (TaskData *)user_data;

    if (local_data->dialog != NULL) {
        gtk_window_present(GTK_WINDOW(local_data->dialog));
        return;
    }

    GtkListBoxRow *selected = gtk_list_box_get_selected_row(local_data->listbox);

    local_data->row_to_delete = selected;

    local_data->dialog = gtk_window_new();
    g_signal_connect(local_data->dialog, "destroy", G_CALLBACK(on_dialog_destroy), local_data);

    gtk_widget_set_size_request(local_data->dialog, 300, 250);

    GtkWidget *box_label = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget *label = gtk_label_new("Are you sure, you want to delete the task?");

    gtk_box_append(GTK_BOX(box_label), label);

    GtkWidget *box_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    gtk_box_append(GTK_BOX(box_label), box_buttons);

    GtkWidget *yesBtn = gtk_button_new_with_label("Yes");
    GtkWidget *noBtn = gtk_button_new_with_label("No");

    gtk_box_append(GTK_BOX(box_buttons), yesBtn);
    gtk_box_append(GTK_BOX(box_buttons), noBtn);

    gtk_widget_set_valign(box_label, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(box_buttons, GTK_ALIGN_CENTER);

    g_signal_connect(yesBtn, "clicked", G_CALLBACK(delete_task_confirmed), local_data);
    g_signal_connect(noBtn, "clicked", G_CALLBACK(delete_task_cancelled), local_data);

    gtk_window_set_child(GTK_WINDOW(local_data->dialog), box_label);
    gtk_window_present(GTK_WINDOW(local_data->dialog));
}

static void add_task(GtkButton *btn, gpointer user_data) {
    TaskData *data = (TaskData *)user_data;

    if (!data || !data->entry || !data->listbox)
        return;

    const char *rowText = gtk_editable_get_text(GTK_EDITABLE(data->entry));

    if (g_utf8_validate(rowText, -1, NULL) && (g_utf8_strlen(rowText, -1)) > 0) {
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *checkbox = gtk_check_button_new();
        GtkWidget *newLabel = gtk_label_new(rowText);

        GtkWidget *box_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_widget_set_halign(box_row, GTK_ALIGN_FILL);

        GtkWidget *box_row_task = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_append(GTK_BOX(box_row), box_row_task);
        gtk_widget_set_hexpand(box_row_task, TRUE);

        gtk_box_append(GTK_BOX(box_row_task), checkbox);
        gtk_box_append(GTK_BOX(box_row_task), newLabel);

        GtkWidget *box_row_menu = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_append(GTK_BOX(box_row), box_row_menu);

        GtkWidget *menuBtn = gtk_menu_button_new();
        gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(menuBtn), "view-more-symbolic");
        gtk_widget_set_halign(menuBtn, GTK_ALIGN_END);
        gtk_box_append(GTK_BOX(box_row_menu), menuBtn);

        GSimpleActionGroup *actions = g_simple_action_group_new();
        GSimpleAction *delete_action = g_simple_action_new("delete", NULL);

        g_signal_connect(delete_action, "activate", G_CALLBACK(delete_task_dialog), data);
        g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(delete_action));

        gtk_widget_insert_action_group(row, "row", G_ACTION_GROUP(actions));
        g_object_unref(actions);

        GMenu *menu = g_menu_new();
        g_menu_append(menu, "Delete", "row.delete");
        gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menuBtn), G_MENU_MODEL(menu));
        g_object_unref(menu);

        g_signal_connect(menuBtn, "notify::active", G_CALLBACK(on_menu_button_active), row);

        g_signal_connect(checkbox, "toggled", G_CALLBACK(complete_task), row);

        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box_row);
        gtk_list_box_append(data->listbox, row);
    }

    gtk_editable_set_text(GTK_EDITABLE(data->entry), "");

    if (data->dialog) {
        gtk_window_destroy(GTK_WINDOW(data->dialog));
        data->dialog = NULL;
    }
}

static void add_task_dialog(GtkButton *btn, gpointer user_data) {
    TaskData *local_data = (TaskData *) user_data;

    if (local_data->dialog != NULL) {
        gtk_window_present(GTK_WINDOW(local_data->dialog));
        return;
    }

    local_data->dialog = gtk_window_new();
    g_signal_connect(local_data->dialog, "destroy", G_CALLBACK(on_dialog_destroy), local_data);

    gtk_widget_set_size_request(local_data->dialog, 200, 150);
    gtk_window_set_title(GTK_WINDOW(local_data->dialog), "Add task");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget *entry = gtk_entry_new();
    local_data->entry = GTK_ENTRY(entry);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter the text");

    GtkWidget *addnewtaskBtn = gtk_button_new_with_label("Add new task");

    gtk_box_append(GTK_BOX(box), entry);
    gtk_box_append(GTK_BOX(box), addnewtaskBtn);

    g_signal_connect(addnewtaskBtn, "clicked", G_CALLBACK(add_task), local_data);

    gtk_window_set_child(GTK_WINDOW(local_data->dialog), box);
    gtk_window_present(GTK_WINDOW(local_data->dialog));
}

static void app_activate(GApplication *app, gpointer *user_data) {
    GtkWidget *window = gtk_application_window_new (GTK_APPLICATION (app));
    gtk_window_set_title (GTK_WINDOW (window), "METANIT.COM");
    gtk_window_set_default_size (GTK_WINDOW (window), 300, 250);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *box_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *box_right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    gtk_box_append(GTK_BOX(box), box_left);
    gtk_box_append(GTK_BOX(box), box_right);

    GtkWidget *addtaskBtn = gtk_button_new_with_label("Add task");
    GtkWidget *todayBtn = gtk_button_new_with_label("Today");
    GtkWidget *inboxBtn = gtk_button_new_with_label("Inbox");

    GtkWidget *entry = gtk_entry_new();

    gtk_box_append(GTK_BOX(box_left), addtaskBtn);
    gtk_box_append(GTK_BOX(box_left), todayBtn);
    gtk_box_append(GTK_BOX(box_left), inboxBtn);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    GtkWidget *searchBtn = gtk_button_new_from_icon_name("folder-saved-search-symbolic");

    gtk_box_append(GTK_BOX(toolbar), searchBtn);

    GtkWidget *tasks = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *listbox = gtk_list_box_new();
    gtk_widget_set_hexpand(listbox, TRUE);

    gtk_box_append(GTK_BOX(tasks), listbox);

    gtk_box_append(GTK_BOX(box_right), toolbar);
    gtk_box_append(GTK_BOX(box_right), tasks);

    TaskData *data = g_new0(TaskData, 1);
    data->listbox = GTK_LIST_BOX(listbox);

    g_signal_connect(addtaskBtn, "clicked", G_CALLBACK(add_task_dialog), data);
    g_signal_connect(searchBtn, "clicked", G_CALLBACK(search_dialog), data);

    gtk_window_set_child (GTK_WINDOW(window), box);
    gtk_window_present (GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(app_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
