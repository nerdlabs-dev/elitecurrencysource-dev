#ifndef RPCCONSOLE_H
#define RPCCONSOLE_H

#include <QDialog>

#include <vector>

namespace Ui {
    class RPCConsole;
}
class ClientModel;

/** Local Bitcoin RPC console. */
class CNodeStats;
class TrafficGraphWidget;
class QLabel;
class QTableWidget;
class QTextBrowser;

class RPCConsole: public QDialog
{
    Q_OBJECT

public:
    explicit RPCConsole(QWidget *parent = 0);
    ~RPCConsole();

    void setClientModel(ClientModel *model);

    enum MessageClass {
        MC_ERROR,
        MC_DEBUG,
        CMD_REQUEST,
        CMD_REPLY,
        CMD_ERROR
    };

protected:
    virtual bool eventFilter(QObject* obj, QEvent *event);

private slots:
    void on_lineEdit_returnPressed();
    void on_tabWidget_currentChanged(int index);
    /** open the debug.log from the current datadir */
    void on_openDebugLogfileButton_clicked();
    /** display messagebox with program parameters (same as bitcoin-qt --help) */
    void on_showCLOptionsButton_clicked();
    /** Peers tab: reload the peer list (only while the tab is shown) */
    void refreshPeers();
    void showPeerDetails();
    /** Network Traffic tab: read the byte totals for the graph */
    void sampleTraffic();

public slots:
    void clear();
    void message(int category, const QString &message, bool html = false);
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count, int countOfPeers);
    /** Go forward or back in history */
    void browseHistory(int offset);
    /** Scroll console view to end */
    void scrollToEnd();
signals:
    // For RPC command executor
    void stopExecutor();
    void cmdRequest(const QString &command);

private:
    Ui::RPCConsole *ui;
    ClientModel *clientModel;
    QStringList history;
    int historyPtr;

    void startExecutor();

    // Peers tab
    QWidget *peersTab;
    QLabel *peerSummary;
    QTableWidget *peerTable;
    QTextBrowser *peerDetails;
    std::vector<CNodeStats> *peerStats;
    QString selectedPeer;
    void setupPeersTab();

    // Network Traffic tab
    QWidget *trafficTab;
    TrafficGraphWidget *trafficGraph;
    QLabel *trafficRecvLabel;
    QLabel *trafficSentLabel;
    void setupTrafficTab();
};

#endif // RPCCONSOLE_H
