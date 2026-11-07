#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"

#include "wallet.h"

#include <QAbstractItemDelegate>
#include <QDesktopServices>
#include <QEvent>
#include <QLocale>
#include <QPainter>
#include <QRadialGradient>
#include <QTextLayout>
#include <QUrl>

// The look: gold on charcoal
static const QColor COLOR_GOLD(221, 182, 111);
static const QColor COLOR_GOLD_DIM(141, 122, 85);
static const QColor COLOR_TEXT_GREY(132, 132, 132);
static const QColor COLOR_ROW_TEXT(112, 112, 112);
static const QColor COLOR_ROW_HOVER(160, 160, 160);
static const QColor COLOR_AMOUNT_OUT(124, 124, 124);

// Height of one row of the recent transactions list; the list shows as many
// rows as fit in the space the window gives it
static const int TX_ROW_HEIGHT = 30;

// Widest the page's content gets; on wider windows it is centred
static const int MAX_CONTENT_WIDTH = 1400;

static const char *DISCORD_URL = "https://discordapp.com/invite/pjdWntg";
static const char *X_URL = "https://twitter.com/elitecurrency";

/** Paints the charcoal background shared by the pages: a dark gradient with a
 *  soft vignette and a fine grain texture */
static void paintThemeBackground(QPainter &painter, const QRect &rect)
{
    QLinearGradient shade(rect.topLeft(), rect.bottomLeft());
    shade.setColorAt(0, QColor(42, 42, 42));
    shade.setColorAt(0.45, QColor(29, 29, 29));
    shade.setColorAt(1, QColor(20, 20, 20));
    painter.fillRect(rect, shade);

    QRadialGradient vignette(rect.center(), qMax(rect.width(), rect.height()) * 0.75);
    vignette.setColorAt(0.55, QColor(0, 0, 0, 0));
    vignette.setColorAt(1, QColor(0, 0, 0, 90));
    painter.fillRect(rect, vignette);

    static const QPixmap grain(":/images/texture");
    painter.drawTiledPixmap(rect, grain);
}

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::BTC)
    {
        font = GUIUtil::themeFont(GUIUtil::FontCondensed, 16, 1);
        font.setItalic(true);
        amountFont = GUIUtil::themeFont(GUIUtil::FontCondensed, 16, 1.5);
        amountFont.setItalic(true);
    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();
        painter->setRenderHint(QPainter::TextAntialiasing);

        QRect rect = option.rect.adjusted(6, 0, -6, 0);
        bool hover = option.state & QStyle::State_MouseOver;
        if (hover)
            painter->fillRect(option.rect, QColor(255, 255, 255, 8));

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();

        // Amount on the right: gold coming in, grey going out
        QString amountText = BitcoinUnits::format(unit, amount, true);
        if (!confirmed)
            amountText = QString("[") + amountText + QString("]");
        QColor amountColor = !confirmed ? COLOR_GOLD_DIM : amount < 0 ? COLOR_AMOUNT_OUT : COLOR_GOLD;
        painter->setFont(amountFont);
        QFontMetrics amountMetrics(amountFont);
        int amountWidth = amountMetrics.boundingRect(amountText).width() + 8;
        painter->setPen(amountColor);
        painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, amountText);

        // Date and address (or label) on the left, shortened to the space left
        painter->setFont(font);
        QFontMetrics metrics(font);
        QString dateText = QLocale().toString(date.date(), QLocale::ShortFormat);
        int dateWidth = metrics.boundingRect(dateText).width() + 12;
        painter->setPen(hover ? COLOR_ROW_HOVER : COLOR_ROW_TEXT);
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, dateText);
        QRect addressRect = rect.adjusted(dateWidth, 0, -amountWidth - 16, 0);
        painter->drawText(addressRect, Qt::AlignLeft | Qt::AlignVCenter,
                          metrics.elidedText(address, Qt::ElideMiddle, addressRect.width()));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(TX_ROW_HEIGHT, TX_ROW_HEIGHT);
    }

    int unit;
    QFont font;
    QFont amountFont;
};
#include "overviewpage.moc"

ScaledImage::ScaledImage(QWidget *parent) : QWidget(parent)
{
}

void ScaledImage::setPixmap(const QPixmap &pixmap)
{
    source = pixmap;
    scaled = QPixmap();
    updateGeometry();
    update();
}

QSize ScaledImage::sizeHint() const
{
    return source.isNull() ? QSize(200, 120) : source.size() / 2;
}

void ScaledImage::resizeEvent(QResizeEvent *event)
{
    scaled = QPixmap();
    QWidget::resizeEvent(event);
}

void ScaledImage::paintEvent(QPaintEvent *event)
{
    if (source.isNull())
        return;
    if (scaled.isNull())
    {
        // Scale once per size, at the screen's pixel density
        qreal ratio = devicePixelRatioF();
        scaled = source.scaled(size() * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        scaled.setDevicePixelRatio(ratio);
    }
    QSizeF drawn = QSizeF(scaled.size()) / scaled.devicePixelRatio();
    QPainter painter(this);
    painter.drawPixmap(QPointF((width() - drawn.width()) / 2, (height() - drawn.height()) / 2), scaled);
}

OrnamentFrame::OrnamentFrame(QWidget *parent) : QFrame(parent)
{
    cornerLeft = QPixmap(":/images/ornament-corner");
    cornerRight = QPixmap::fromImage(cornerLeft.toImage().mirrored(true, false));
}

void OrnamentFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // The panel sits inside the ornaments, which reach out over its corners
    int inset = cornerLeft.isNull() ? 0 : cornerLeft.width() / 4;
    QRectF panel = QRectF(rect()).adjusted(inset, inset * 1.6, -inset, 0);
    QLinearGradient fill(panel.topLeft(), panel.bottomLeft());
    fill.setColorAt(0, QColor(18, 18, 18, 235));
    fill.setColorAt(1, QColor(20, 20, 20, 200));
    painter.setPen(QPen(QColor(255, 255, 255, 10), 1));
    painter.setBrush(fill);
    painter.drawRoundedRect(panel, 3, 3);

    painter.drawPixmap(0, 0, cornerLeft);
    painter.drawPixmap(width() - cornerRight.width(), 0, cornerRight);
}

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    model(0),
    currentBalance(-1),
    currentStake(0),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    encryptionStatus(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    ui->setupUi(this);

    // Fonts (bundled with the wallet, see GUIUtil::loadThemeFonts)
    QFont statFont = GUIUtil::themeFont(GUIUtil::FontCondensed, 18, 0.5);
    QList<QLabel*> statNames, statValues;
    statNames << ui->labelStakeText << ui->labelImmatureText << ui->labelUnconfirmedText << ui->labelNumTransactionsText;
    statValues << ui->labelStake << ui->labelImmature << ui->labelUnconfirmed << ui->labelNumTransactions;
    foreach (QLabel *label, statNames + statValues)
        label->setFont(statFont);
    ui->labelStatusTitle->setFont(GUIUtil::themeFont(GUIUtil::FontCondensed, 22, 3));
    ui->labelRoyalty->setFont(GUIUtil::themeFont(GUIUtil::FontScript, 50));
    QFont aboutFont = GUIUtil::themeFont(GUIUtil::FontCondensed, 14, 0.3);
    aboutFont.setItalic(true);
    ui->labelAbout->setFont(aboutFont);
    ui->labelRecentTitle->setFont(GUIUtil::themeFont(GUIUtil::FontScript, 32));
    QFont syncFont = GUIUtil::themeFont(GUIUtil::FontCondensed, 13, 1);
    syncFont.setItalic(true);
    ui->labelSyncStatus->setFont(syncFont);
    ui->labelBalanceTitle->setFont(GUIUtil::themeFont(GUIUtil::FontScript, 38));
    ui->labelBalance->setFont(GUIUtil::themeFont(GUIUtil::FontCondensed, 44, 4));
    ui->buttonLock->setFont(GUIUtil::themeFont(GUIUtil::FontCondensed, 16, 6));

    // Colours. These are set here rather than in the theme file, so the page
    // keeps its look (it draws its own background) with the theme turned off.
    setStyleSheet(QString(
        "QLabel { background: transparent; color: %1; }"
        "#labelStake, #labelImmature, #labelUnconfirmed, #labelNumTransactions { color: %2; }"
        "#labelAbout { color: %2; }"
        "#labelSyncStatus { color: %3; }"
        "QListView { background: transparent; border: none; }"
        "QToolButton { background: transparent; border: none; padding: 2px; }"
        "QToolButton:hover { background: rgba(221, 182, 111, 25); border-radius: 4px; }"
        "#buttonLock { color: %4; background: #121212; border: 1px solid #262626; border-radius: 17px;"
        "  min-height: 32px; max-height: 32px; padding: 0 30px; }"
        "#buttonLock:hover { color: %1; border-color: %3; }"
        "#buttonLock:pressed { background: #0c0c0c; }")
        .arg(COLOR_GOLD.name(), COLOR_TEXT_GREY.name(), COLOR_GOLD_DIM.name(), QColor(110, 110, 110).name()));

    ui->crest->setPixmap(QPixmap(":/images/wings-logo"));
    ui->buttonDiscord->setIcon(QIcon(":/images/social-discord"));
    ui->buttonX->setIcon(QIcon(":/images/social-x"));
    connect(ui->buttonDiscord, SIGNAL(clicked()), this, SLOT(openDiscord()));
    connect(ui->buttonX, SIGNAL(clicked()), this, SLOT(openX()));

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setMinimumHeight(2 * TX_ROW_HEIGHT);
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listTransactions->setMouseTracking(true);
    ui->listTransactions->viewport()->setAttribute(Qt::WA_Hover, true);
    ui->listTransactions->viewport()->installEventFilter(this);
    aboutText = ui->labelAbout->text();
    ui->labelAbout->installEventFilter(this);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));
    connect(ui->buttonLock, SIGNAL(clicked()), this, SLOT(lockButtonClicked()));
    ui->buttonLock->hide(); // until there is a wallet

    showOutOfSyncWarning(true);
}

void OverviewPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    paintThemeBackground(painter, rect());
}

bool OverviewPage::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->listTransactions->viewport() && event->type() == QEvent::Resize)
        updateTransactionRows();
    if (object == ui->labelAbout && event->type() == QEvent::Resize)
        fitAboutText();
    return QWidget::eventFilter(object, event);
}

void OverviewPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // On a wide window keep the page's content together in the middle
    int side = qMax(28, (width() - MAX_CONTENT_WIDTH) / 2);
    ui->mainLayout->setContentsMargins(side, 4, side, 0);
}

void OverviewPage::fitAboutText()
{
    // Show as many whole lines of the description as there is room for,
    // ending with an ellipsis if it doesn't all fit
    QLabel *label = ui->labelAbout;
    QFontMetrics metrics(label->font());
    int lineHeight = metrics.lineSpacing();
    int maxLines = label->height() / lineHeight;
    QTextLayout layout(aboutText, label->font());
    layout.beginLayout();
    QStringList lines;
    bool complete = false;
    for (;;)
    {
        QTextLine line = layout.createLine();
        if (!line.isValid())
        {
            complete = true;
            break;
        }
        line.setLineWidth(label->width());
        if (lines.size() == maxLines)
            break;
        lines << aboutText.mid(line.textStart(), line.textLength());
    }
    layout.endLayout();
    if (!complete && !lines.isEmpty())
        lines.last() = metrics.elidedText(lines.last().trimmed() + QString(" ") + QChar(0x2026),
                                          Qt::ElideRight, label->width());
    QString text = complete ? aboutText : lines.join("").trimmed();
    if (label->text() != text)
        label->setText(text);
}

void OverviewPage::updateTransactionRows()
{
    // As many transactions as there is room for
    if (!filter)
        return;
    int rows = qMax(1, ui->listTransactions->viewport()->height() / TX_ROW_HEIGHT);
    filter->setLimit(rows);
    filter->invalidate();
}

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    // The balance in large figures, with the unit after it in smaller type
    ui->labelBalance->setText(BitcoinUnits::format(unit, balance) +
                              QString("<span style=\"font-size:20px;\"> %1</span>").arg(BitcoinUnits::name(unit)));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
}

void OverviewPage::setNumTransactions(int count)
{
    ui->labelNumTransactions->setText(QLocale::system().toString(count));
}

void OverviewPage::setEncryptionStatus(int status)
{
    encryptionStatus = status;
    switch (status)
    {
    case WalletModel::Unencrypted:
        ui->buttonLock->setText(tr("NOT ENCRYPTED"));
        ui->buttonLock->setToolTip(tr("Your wallet is not encrypted. Click to protect it with a passphrase."));
        break;
    case WalletModel::Unlocked:
        ui->buttonLock->setText(fWalletUnlockStakingOnly ? tr("UNLOCKED FOR STAKING") : tr("UNLOCKED"));
        ui->buttonLock->setToolTip(tr("Your wallet is unlocked. Click to lock it."));
        break;
    case WalletModel::Locked:
    default:
        ui->buttonLock->setText(tr("LOCKED"));
        ui->buttonLock->setToolTip(tr("Your wallet is locked. Click to unlock it for staking."));
        break;
    }
    ui->buttonLock->show();
}

void OverviewPage::lockButtonClicked()
{
    switch (encryptionStatus)
    {
    case WalletModel::Unencrypted: emit encryptWalletClicked(); break;
    case WalletModel::Unlocked: emit lockWalletClicked(); break;
    case WalletModel::Locked: emit unlockWalletClicked(); break;
    }
    if (model)
        setEncryptionStatus(model->getEncryptionStatus());
}

void OverviewPage::openDiscord()
{
    QDesktopServices::openUrl(QUrl(DISCORD_URL));
}

void OverviewPage::openX()
{
    QDesktopServices::openUrl(QUrl(X_URL));
}

void OverviewPage::setModel(WalletModel *model)
{
    this->model = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy(this);
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);
        updateTransactionRows();

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        setNumTransactions(model->getNumTransactions());
        connect(model, SIGNAL(numTransactionsChanged(int)), this, SLOT(setNumTransactions(int)));

        setEncryptionStatus(model->getEncryptionStatus());
        connect(model, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("1337")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, model->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = model->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelSyncStatus->setVisible(fShow);
}
