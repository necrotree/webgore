#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QLineEdit>
#include <QMainWindow>
#include <QProgressBar>
#include <QStyle>
#include <QToolBar>
#include <QUrl>
#include <QWebView>

class BrowserWindow final : public QMainWindow {
public:
    BrowserWindow() {
        setWindowTitle("Webgore");
        resize(1280, 800);

        webView_ = new QWebView(this);
        setCentralWidget(webView_);

        auto *toolbar = addToolBar("Navigation");
        toolbar->setMovable(false);

        auto *backAction = toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), "Back");
        auto *forwardAction = toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), "Forward");
        auto *reloadAction = toolbar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), "Reload");
        auto *homeAction = toolbar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon), "Home");

        addressBar_ = new QLineEdit(this);
        addressBar_->setClearButtonEnabled(true);
        addressBar_->setPlaceholderText("Search or enter address");
        toolbar->addWidget(addressBar_);

        progressBar_ = new QProgressBar(this);
        progressBar_->setMaximumWidth(120);
        progressBar_->setTextVisible(false);
        progressBar_->hide();
        toolbar->addWidget(progressBar_);

        connect(backAction, &QAction::triggered, webView_, &QWebView::back);
        connect(forwardAction, &QAction::triggered, webView_, &QWebView::forward);
        connect(reloadAction, &QAction::triggered, webView_, &QWebView::reload);
        connect(homeAction, &QAction::triggered, this, [this] { navigateTo(homeUrl()); });
        connect(addressBar_, &QLineEdit::returnPressed, this, [this] {
            navigateTo(QUrl::fromUserInput(addressBar_->text()));
        });
        connect(webView_, &QWebView::urlChanged, this, [this](const QUrl &url) {
            addressBar_->setText(url.toDisplayString());
        });
        connect(webView_, &QWebView::titleChanged, this, [this](const QString &title) {
            setWindowTitle(title.isEmpty() ? "Webgore" : title + " - Webgore");
        });
        connect(webView_, &QWebView::loadProgress, this, [this](int progress) {
            progressBar_->setValue(progress);
            progressBar_->setVisible(progress < 100);
        });

        navigateTo(homeUrl());
    }

private:
    static QUrl homeUrl() {
        return QUrl("https://www.google.com");
    }

    void navigateTo(const QUrl &url) {
        if (url.isValid()) {
            webView_->load(url);
        }
    }

    QLineEdit *addressBar_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
    QWebView *webView_ = nullptr;
};

int main(int argc, char *argv[]) {
    QApplication application(argc, argv);

    BrowserWindow browser;
    browser.show();

    return application.exec();
}