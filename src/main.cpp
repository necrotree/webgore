#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QLabel>
#include <QKeySequence>
#include <QLineEdit>
#include <QMainWindow>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QProgressBar>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QStyle>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QWebPage>
#include <QWebView>
#include <QWebSettings>

class BrowserPage final : public QWebPage {
protected:
    QString userAgentForUrl(const QUrl &) const override {
        return QStringLiteral(
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
            "AppleWebKit/537.36 (KHTML, like Gecko) "
            "Chrome/126.0.0.0 Safari/537.36");
    }
};

class BrowserWindow final : public QMainWindow {
public:
    BrowserWindow() {
        setWindowTitle("Webgore");
        resize(1360, 860);
        setStyleSheet(R"(
            QMainWindow {
                background: #fbfbfd;
            }
            QToolBar#browserToolbar {
                background: #ffffff;
                border: none;
                border-bottom: 1px solid #e5e5ea;
                padding: 10px 18px;
                spacing: 8px;
            }
            QLabel#productName {
                color: #1d1d1f;
                font-size: 18px;
                font-weight: 600;
                padding-right: 14px;
            }
            QTabWidget::pane {
                border: none;
            }
            QTabBar {
                background: #ffffff;
                border: none;
                padding: 8px 18px 0 18px;
            }
            QTabBar::tab {
                background: #f2f2f7;
                border: none;
                border-radius: 7px 7px 0 0;
                color: #6e6e73;
                min-width: 150px;
                padding: 8px 28px 8px 12px;
            }
            QTabBar::tab:selected {
                background: #ffffff;
                color: #1d1d1f;
                border-top: 2px solid #fa243c;
            }
            QTabBar::close-button {
                image: none;
                subcontrol-position: right;
            }
            QTabBar::close-button:hover {
                background: #d1d1d6;
                border-radius: 6px;
            }
            QToolButton {
                background: rgba(255, 255, 255, 168);
                border: 1px solid rgba(255, 255, 255, 210);
                border-radius: 9px;
                padding: 5px;
            }
            QToolButton:hover {
                background: rgba(255, 255, 255, 238);
                border: 1px solid #e1e1e7;
            }
            QToolButton:pressed {
                background: #e8e8ed;
            }
            QToolButton#newTabButton {
                color: #fa243c;
                font-size: 20px;
                font-weight: 500;
            }
            QToolButton#newTabButton:hover {
                background: #ffe9ec;
            }
            QLineEdit#addressBar {
                background: #f2f2f7;
                border: 1px solid transparent;
                border-radius: 10px;
                color: #1d1d1f;
                min-height: 30px;
                padding: 0 12px;
                selection-background-color: #007aff;
            }
            QLineEdit#addressBar:focus {
                background: #ffffff;
                border: 1px solid #fa243c;
            }
            QProgressBar#loadProgress {
                background: transparent;
                border: none;
                max-height: 3px;
            }
            QProgressBar#loadProgress::chunk {
                background: #fa243c;
                border-radius: 1px;
            }
        )");

        tabs_ = new QTabWidget(this);
        tabs_->setDocumentMode(true);
        tabs_->setTabsClosable(true);
        tabs_->setMovable(true);
        setCentralWidget(tabs_);

        auto *toolbar = addToolBar("Navigation");
        toolbar->setObjectName("browserToolbar");
        toolbar->setMovable(false);
        toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

        auto *productName = new QLabel("Webgore", this);
        productName->setObjectName("productName");
        toolbar->addWidget(productName);

        auto *backAction = toolbar->addAction(navigationIcon(NavigationIcon::Back), "Back");
        auto *forwardAction = toolbar->addAction(navigationIcon(NavigationIcon::Forward), "Forward");
        auto *reloadAction = toolbar->addAction(navigationIcon(NavigationIcon::Reload), "Reload");
        auto *homeAction = toolbar->addAction(navigationIcon(NavigationIcon::Home), "Home");
        backAction->setToolTip("Back");
        forwardAction->setToolTip("Forward");
        reloadAction->setToolTip("Reload");
        homeAction->setToolTip("Home");
        backAction->setShortcut(QKeySequence("Alt+Left"));
        forwardAction->setShortcut(QKeySequence("Alt+Right"));
        reloadAction->setShortcut(QKeySequence("Ctrl+R"));
        homeAction->setShortcut(QKeySequence("Alt+Home"));

        addressBar_ = new QLineEdit(this);
        addressBar_->setObjectName("addressBar");
        addressBar_->setClearButtonEnabled(true);
        addressBar_->setPlaceholderText("Search or enter address");
        addressBar_->setMinimumWidth(520);
        toolbar->addWidget(addressBar_);

        auto *newTabButton = new QToolButton(this);
        newTabButton->setObjectName("newTabButton");
        newTabButton->setText("+");
        newTabButton->setToolTip("New tab");
        newTabButton->setFixedSize(30, 30);
        toolbar->addWidget(newTabButton);

        progressBar_ = new QProgressBar(this);
        progressBar_->setObjectName("loadProgress");
        progressBar_->setMaximumWidth(120);
        progressBar_->setTextVisible(false);
        progressBar_->hide();
        toolbar->addWidget(progressBar_);

        connect(backAction, &QAction::triggered, this, [this] { currentWebView()->back(); });
        connect(forwardAction, &QAction::triggered, this, [this] { currentWebView()->forward(); });
        connect(reloadAction, &QAction::triggered, this, [this] { currentWebView()->reload(); });
        connect(homeAction, &QAction::triggered, this, [this] { navigateTo(homeUrl()); });
        connect(newTabButton, &QToolButton::clicked, this, [this] { createTab(homeUrl()); });
        connect(addressBar_, &QLineEdit::returnPressed, this, [this] {
            navigateTo(QUrl::fromUserInput(addressBar_->text()));
        });
        connect(tabs_, &QTabWidget::currentChanged, this, [this](int) {
            updateChrome();
        });
        connect(tabs_, &QTabWidget::tabCloseRequested, this, [this](int index) {
            closeTab(index);
        });

        addShortcut("Ctrl+T", [this] { createTab(homeUrl()); });
        addShortcut("Ctrl+W", [this] { closeTab(tabs_->currentIndex()); });
        addShortcut("Ctrl+L", [this] {
            addressBar_->setFocus();
            addressBar_->selectAll();
        });
        addShortcut("Ctrl+Tab", [this] {
            tabs_->setCurrentIndex((tabs_->currentIndex() + 1) % tabs_->count());
        });
        addShortcut("Ctrl+Shift+Tab", [this] {
            tabs_->setCurrentIndex((tabs_->currentIndex() + tabs_->count() - 1) % tabs_->count());
        });
        addShortcut("Ctrl++", [this] { currentWebView()->setZoomFactor(currentWebView()->zoomFactor() + 0.1); });
        addShortcut("Ctrl+-", [this] { currentWebView()->setZoomFactor(currentWebView()->zoomFactor() - 0.1); });
        addShortcut("Ctrl+0", [this] { currentWebView()->setZoomFactor(1.0); });

        createTab(homeUrl());
    }

private:
    enum class NavigationIcon {
        Back,
        Forward,
        Reload,
        Home,
    };

    static QIcon navigationIcon(NavigationIcon type) {
        QPixmap pixmap(22, 22);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        QPen pen(QColor("#35353a"), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        if (type == NavigationIcon::Back || type == NavigationIcon::Forward) {
            QPainterPath path;
            if (type == NavigationIcon::Back) {
                path.moveTo(14.0, 4.5);
                path.lineTo(8.0, 11.0);
                path.lineTo(14.0, 17.5);
            } else {
                path.moveTo(8.0, 4.5);
                path.lineTo(14.0, 11.0);
                path.lineTo(8.0, 17.5);
            }
            painter.drawPath(path);
        } else if (type == NavigationIcon::Reload) {
            QPainterPath path;
            path.arcMoveTo(5, 5, 12, 12, 35);
            path.arcTo(5, 5, 12, 12, 35, 285);
            painter.drawPath(path);
            painter.drawLine(16.5, 5.0, 16.5, 9.0);
            painter.drawLine(16.5, 5.0, 12.5, 5.0);
        } else {
            QPainterPath path;
            path.moveTo(4.5, 10.5);
            path.lineTo(11.0, 4.8);
            path.lineTo(17.5, 10.5);
            path.lineTo(16.0, 10.5);
            path.lineTo(16.0, 17.0);
            path.lineTo(6.0, 17.0);
            path.lineTo(6.0, 10.5);
            path.closeSubpath();
            painter.drawPath(path);
        }

        return QIcon(pixmap);
    }

    static QUrl homeUrl() {
        return QUrl("https://www.google.com");
    }

    void addShortcut(const char *shortcut, const std::function<void()> &handler) {
        addShortcut(QKeySequence(QString::fromLatin1(shortcut)), handler);
    }

    void addShortcut(const QKeySequence &shortcut, const std::function<void()> &handler) {
        auto *action = new QAction(this);
        action->setShortcut(shortcut);
        action->setShortcutContext(Qt::WindowShortcut);
        connect(action, &QAction::triggered, this, handler);
        addAction(action);
    }

    void closeTab(int index) {
        if (tabs_->count() == 1) {
            currentWebView()->load(homeUrl());
            return;
        }

        auto *closingPage = tabs_->widget(index);
        tabs_->removeTab(index);
        closingPage->deleteLater();
    }

    void navigateTo(const QUrl &url) {
        if (url.isValid()) {
            currentWebView()->load(url);
        }
    }

    QWebView *currentWebView() const {
        return qobject_cast<QWebView *>(tabs_->currentWidget());
    }

    void createTab(const QUrl &url) {
        auto *webView = new QWebView(tabs_);
        webView->setPage(new BrowserPage());
        const int index = tabs_->addTab(webView, "New Tab");
        tabs_->setCurrentIndex(index);

        connect(webView, &QWebView::urlChanged, this, [this, webView](const QUrl &) {
            if (webView == currentWebView()) {
                updateChrome();
            }
        });
        connect(webView, &QWebView::titleChanged, this, [this, webView](const QString &title) {
            const int tabIndex = tabs_->indexOf(webView);
            if (tabIndex >= 0) {
                tabs_->setTabText(tabIndex, title.isEmpty() ? "New Tab" : title.left(28));
            }
            if (webView == currentWebView()) {
                updateChrome();
            }
        });
        connect(webView, &QWebView::loadProgress, this, [this, webView](int progress) {
            if (webView == currentWebView()) {
                progressBar_->setValue(progress);
                progressBar_->setVisible(progress < 100);
            }
        });

        webView->load(url);
    }

    void updateChrome() {
        auto *webView = currentWebView();
        if (webView == nullptr) {
            return;
        }

        addressBar_->setText(webView->url().toDisplayString());
        const QString title = webView->title();
        setWindowTitle(title.isEmpty() ? "Webgore" : title + " - Webgore");
    }

    QLineEdit *addressBar_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
    QTabWidget *tabs_ = nullptr;
};

int main(int argc, char *argv[]) {
    QApplication application(argc, argv);

    QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
    sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);
    QSslConfiguration::setDefaultConfiguration(sslConfiguration);

    QWebSettings *webSettings = QWebSettings::globalSettings();
    webSettings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
    webSettings->setAttribute(QWebSettings::Accelerated2dCanvasEnabled, true);
    webSettings->setAttribute(QWebSettings::WebGLEnabled, true);
    webSettings->setAttribute(QWebSettings::MediaSourceEnabled, true);
    webSettings->setAttribute(QWebSettings::MediaEnabled, true);
    webSettings->setAttribute(QWebSettings::WebAudioEnabled, true);
    webSettings->setAttribute(QWebSettings::FullScreenSupportEnabled, true);

    BrowserWindow browser;
    browser.show();

    return application.exec();
}