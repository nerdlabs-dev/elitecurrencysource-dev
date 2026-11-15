// Copyright (c) 2026 Elite Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// APR Tracking > Interest Estimate: what an amount of 1337 earns from staking.

#include "calcdialog.h"
#include "ui_calcdialog.h"

#include "main.h"
#include "util.h"

#include <QString>
#include <QPushButton>
#include <QDialogButtonBox>

calcDialog::calcDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::calcDialog)
{
    ui->setupUi(this);

    // Enter calculates; only clicking Close (or Esc) closes the window
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(pushButtonClicked()));
    QPushButton *btnClose = ui->buttonBox->button(QDialogButtonBox::Close);
    if (btnClose)
    {
        btnClose->setAutoDefault(false);
        btnClose->setDefault(false);
    }
    ui->pushButton->setDefault(true);
    ui->blockSizeEdit->setFocus();
}

calcDialog::~calcDialog()
{
    delete ui;
}

void calcDialog::setModel(ClientModel *model)
{
}

// Uses the reward rule the network enforces (GetProofOfStakeReward in
// main.cpp): reward = coin-days x yearly rate / 365.24, at today's rate.
void calcDialog::pushButtonClicked()
{
    bool fOk = false;
    double dAmount = ui->blockSizeEdit->text().trimmed().remove(',').toDouble(&fOk);
    if (!fOk || dAmount < 0 || dAmount > (double)MAX_MONEY / COIN)
    {
        ui->yearlyAPR->setText("-");
        ui->dayResult->setText(tr("Enter an amount of 1337"));
        return;
    }

    // Reward for COIN coin-days (a large round number, so integer rounding
    // in the reward rule doesn't matter), turned into coins per coin-day
    int64_t nReward = GetProofOfStakeReward(COIN, 0, (unsigned int)GetAdjustedTime());
    double dPerCoinDay = (double)nReward / COIN / COIN;
    double dRate = dPerCoinDay * 365.2424 * 100.0;   // percent per year

    double dDaily = dAmount * dPerCoinDay;
    ui->yearlyAPR->setText(QString::number(dRate, 'f', 3) + " %");
    ui->dayResult->setText(tr("%1 1337 a day, about %2 a month and %3 a year")
                           .arg(QString::number(dDaily, 'f', 6))
                           .arg(QString::number(dDaily * 30.44, 'f', 4))
                           .arg(QString::number(dDaily * 365.2424, 'f', 4)));
}
