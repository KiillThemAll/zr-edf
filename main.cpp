#include <QProcessEnvironment>
#include <QDebug>

#include "astraledf.h"

int main(int argc, char **argv)
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString accessToken = env.value("ASTRAL_EDF_TOKEN");
    QString abonentId = env.value("ASTRAL_EDF_ABONENT_ID");

    if (accessToken.isEmpty() || abonentId.isEmpty())
    {
        qDebug() << "ASTRAL_EDF_TOKEN and ASTRAL_EDF_ABONENT_ID env variables are required";
        return -1;
    }

    AstralEdf::globalApi(accessToken.toStdString(), abonentId.toStdString());

    AbstractEdfDriver::ContactsPage contactsPage = AstralEdf::globalApi().fetchCounterparties(QString(), 0, 15);
    qDebug() << "EDF counterparties count:" << contactsPage.count;

    return 0;
}

