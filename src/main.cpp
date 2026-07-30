#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QLineEdit>
#include <QMainWindow>
#include <QProgressBar>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QStyle>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QWebView>

class BrowserWindow final : public QMainWindow {
public:
    BrowserWindow() {
        setWindowTitle("Webgore");
        resize(1360, 860);
        setStyleSheet(R"(
            QMainWindow {
                background: #f5f5f7;
            }
            QToolBar#browserToolbar {
                background: #f5f5f7;
                border: none;
                border-bottom: 1px solid #dbdbe0;
                padding: 8px 14px;
                spacing: 6px;
            }
            QTabWidget::pane {
                border: none;
            }
            QTabBar {
                background: #ececef;
                border: none;
                padding: 7px 12px 0 12px;
            }
            QTabBar::tab {
                background: transparent;
                border: none;
                border-radius: 8px 8px 0 0;
                color: #606067;
                min-width: 150px;
                padding: 8px 28px 8px 12px;
            }
            QTabBar::tab:selected {
                background: #f5f5f7;
                color: #1d1d1f;
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
                background: transparent;
                border: none;
                border-radius: 7px;
                padding: 6px;
            }
            QToolButton:hover {
                background: #dedee3;
            }
            QToolButton:pressed {
                background: #c9c9d0;
            }
            QLineEdit#addressBar {
                background: #ffffff;
                border: 1px solid #d8d8de;
                border-radius: 10px;
                color: #1d1d1f;
                min-height: 30px;
                padding: 0 12px;
                selection-background-color: #007aff;
            }
            QLineEdit#addressBar:focus {
                border: 1px solid #007aff;
            }
            QProgressBar#loadProgress {
                background: transparent;
                border: none;
                max-height: 3px;
            }
            QProgressBar#loadProgress::chunk {
                background: #007aff;
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

        auto *backAction = toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), "Back");
        auto *forwardAction = toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), "Forward");
        auto *reloadAction = toolbar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), "Reload");
        auto *homeAction = toolbar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon), "Home");
        backAction->setToolTip("Back");
        forwardAction->setToolTip("Forward");
        reloadAction->setToolTip("Reload");
        homeAction->setToolTip("Home");

        addressBar_ = new QLineEdit(this);
        addressBar_->setObjectName("addressBar");
        addressBar_->setClearButtonEnabled(true);
        addressBar_->setPlaceholderText("Search or enter address");
        addressBar_->setMinimumWidth(520);
        toolbar->addWidget(addressBar_);

        auto *newTabButton = new QToolButton(this);
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
            if (tabs_->count() == 1) {
                currentWebView()->load(homeUrl());
                return;
            }

            auto *closingPage = tabs_->widget(index);
            tabs_->removeTab(index);
            closingPage->deleteLater();
        });

        createTab(homeUrl());
    }

private:
    static QUrl homeUrl() {
        return QUrl("https://www.google.com");
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

    BrowserWindow browser;
    browser.show();

    return application.exec();
}