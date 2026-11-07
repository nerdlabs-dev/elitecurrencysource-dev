#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>
#include <QFrame>
#include <QPixmap>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
    class OverviewPage;
}
class WalletModel;
class TxViewDelegate;
class TransactionFilterProxy;

/** An image drawn as large as fits, keeping its shape (the crest) */
class ScaledImage : public QWidget
{
public:
    explicit ScaledImage(QWidget *parent = 0);
    void setPixmap(const QPixmap &pixmap);
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QPixmap source;
    QPixmap scaled; // cached for the current size
};

/** The dark panel behind the recent transactions, with gold ornaments on its top corners */
class OrnamentFrame : public QFrame
{
public:
    explicit OrnamentFrame(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPixmap cornerLeft;
    QPixmap cornerRight;
};

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = 0);
    ~OverviewPage();

    void setModel(WalletModel *model);
    void showOutOfSyncWarning(bool fShow);

public slots:
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance);
    void setNumTransactions(int count);
    void setEncryptionStatus(int status);

signals:
    void transactionClicked(const QModelIndex &index);
    // The lock button: what it asks the main window to do
    void unlockWalletClicked();
    void lockWalletClicked();
    void encryptWalletClicked();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::OverviewPage *ui;
    WalletModel *model;
    qint64 currentBalance;
    qint64 currentStake;
    qint64 currentUnconfirmedBalance;
    qint64 currentImmatureBalance;
    int encryptionStatus;
    QString aboutText; // the description, shortened to fit by fitAboutText

    TxViewDelegate *txdelegate;
    TransactionFilterProxy *filter;

    void updateTransactionRows();
    void fitAboutText();

private slots:
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex &index);
    void lockButtonClicked();
    void openDiscord();
    void openX();
};

#endif // OVERVIEWPAGE_H
