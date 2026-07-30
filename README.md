# webgore

A Windows browser made with WebKit.

Webgore embeds the **WebKit2** rendering engine (via GTK + PyGObject) into a
native-feeling desktop window.  It ships a URL bar, back/forward/reload/home
navigation, smart URL handling (auto-prefixes `https://` and falls back to a
DuckDuckGo search), and a status bar.

---

## Requirements

| Requirement | Version |
|-------------|---------|
| Python      | ≥ 3.10  |
| GTK         | ≥ 3.24  |
| WebKit2GTK  | ≥ 2.38  |
| PyGObject   | ≥ 3.44  |

### Windows — install via MSYS2

1. Download and install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 MinGW 64-bit** shell and run:

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-python \
          mingw-w64-x86_64-python-gobject \
          mingw-w64-x86_64-webkitgtk3
```

3. Install the remaining Python dependencies:

```bash
pip install -r requirements.txt
```

---

## Running

```bash
python main.py
```

---

## Project layout

```
webgore/
├── main.py          # entry point
├── requirements.txt
└── webgore/
    ├── __init__.py
    ├── browser.py   # BrowserWindow (GTK window + WebKit2 WebView)
    └── constants.py # app-wide defaults
```

---

## License

MIT — see [LICENSE](LICENSE).
