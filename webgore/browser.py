import gi

gi.require_version('Gtk', '3.0')
gi.require_version('WebKit2', '4.0')

from gi.repository import Gtk, WebKit2

from .constants import APP_NAME, DEFAULT_HOME, DEFAULT_HEIGHT, DEFAULT_WIDTH


class BrowserWindow(Gtk.Window):
    """Main browser window backed by a WebKit2 WebView."""

    def __init__(self):
        super().__init__(title=APP_NAME)
        self.set_default_size(DEFAULT_WIDTH, DEFAULT_HEIGHT)

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.add(vbox)

        toolbar = self._build_toolbar()
        vbox.pack_start(toolbar, False, False, 0)

        self.webview = WebKit2.WebView()
        self.webview.connect('load-changed', self._on_load_changed)
        self.webview.connect('load-failed', self._on_load_failed)
        self.webview.connect('notify::title', self._on_title_changed)
        self.webview.connect('notify::uri', self._on_uri_changed)

        scroll = Gtk.ScrolledWindow()
        scroll.add(self.webview)
        vbox.pack_start(scroll, True, True, 0)

        self.statusbar = Gtk.Statusbar()
        self._status_ctx = self.statusbar.get_context_id(APP_NAME)
        vbox.pack_start(self.statusbar, False, False, 0)

        self.webview.load_uri(DEFAULT_HOME)

    # ------------------------------------------------------------------
    # Toolbar construction
    # ------------------------------------------------------------------

    def _build_toolbar(self):
        toolbar = Gtk.Toolbar()
        toolbar.get_style_context().add_class(Gtk.STYLE_CLASS_PRIMARY_TOOLBAR)

        self.back_btn = self._tool_button('go-previous', 'Back', self._on_back)
        self.forward_btn = self._tool_button('go-next', 'Forward', self._on_forward)
        self.reload_btn = self._tool_button('view-refresh', 'Reload', self._on_reload)
        home_btn = self._tool_button('go-home', 'Home', self._on_home)

        for btn in (self.back_btn, self.forward_btn, self.reload_btn, home_btn):
            toolbar.insert(btn, -1)

        toolbar.insert(Gtk.SeparatorToolItem(), -1)

        self.url_entry = Gtk.Entry()
        self.url_entry.set_placeholder_text('Enter URL or search term…')
        self.url_entry.connect('activate', self._on_url_activate)

        url_item = Gtk.ToolItem()
        url_item.set_expand(True)
        url_item.add(self.url_entry)
        toolbar.insert(url_item, -1)

        go_btn = self._tool_button('go-jump', 'Go', lambda _: self._navigate(self.url_entry.get_text()))
        toolbar.insert(go_btn, -1)

        return toolbar

    @staticmethod
    def _tool_button(icon_name, label, callback):
        btn = Gtk.ToolButton()
        btn.set_icon_name(icon_name)
        btn.set_tooltip_text(label)
        btn.connect('clicked', callback)
        return btn

    # ------------------------------------------------------------------
    # Navigation helpers
    # ------------------------------------------------------------------

    def _navigate(self, text):
        """Load *text* as a URI, adding a scheme or falling back to search."""
        text = text.strip()
        if not text:
            return
        if text.startswith(('http://', 'https://', 'ftp://', 'file://')):
            uri = text
        elif '.' in text and ' ' not in text:
            uri = 'https://' + text
        else:
            uri = 'https://duckduckgo.com/?q=' + text.replace(' ', '+')
        self.webview.load_uri(uri)

    # ------------------------------------------------------------------
    # Toolbar callbacks
    # ------------------------------------------------------------------

    def _on_back(self, _widget):
        if self.webview.can_go_back():
            self.webview.go_back()

    def _on_forward(self, _widget):
        if self.webview.can_go_forward():
            self.webview.go_forward()

    def _on_reload(self, _widget):
        if self.webview.is_loading():
            self.webview.stop_loading()
        else:
            self.webview.reload()

    def _on_home(self, _widget):
        self.webview.load_uri(DEFAULT_HOME)

    def _on_url_activate(self, entry):
        self._navigate(entry.get_text())

    # ------------------------------------------------------------------
    # WebView signal handlers
    # ------------------------------------------------------------------

    def _on_load_changed(self, webview, event):
        if event == WebKit2.LoadEvent.STARTED:
            self._set_status('Loading…')
            self.reload_btn.set_icon_name('process-stop')
        elif event == WebKit2.LoadEvent.COMMITTED:
            self._set_status('Receiving data…')
        elif event == WebKit2.LoadEvent.FINISHED:
            self._set_status('Done')
            self.reload_btn.set_icon_name('view-refresh')
        self._refresh_nav_buttons()

    def _on_load_failed(self, _webview, _event, _uri, error):
        self._set_status(f'Error: {error.message}')
        self.reload_btn.set_icon_name('view-refresh')
        return False

    def _on_title_changed(self, webview, _param):
        title = webview.get_title()
        self.set_title(f'{title} — {APP_NAME}' if title else APP_NAME)

    def _on_uri_changed(self, webview, _param):
        uri = webview.get_uri()
        if uri and uri != self.url_entry.get_text():
            self.url_entry.set_text(uri)

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    def _refresh_nav_buttons(self):
        self.back_btn.set_sensitive(self.webview.can_go_back())
        self.forward_btn.set_sensitive(self.webview.can_go_forward())

    def _set_status(self, message):
        self.statusbar.pop(self._status_ctx)
        self.statusbar.push(self._status_ctx, message)
