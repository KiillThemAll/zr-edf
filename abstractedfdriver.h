#ifndef ABSTRACTEDFDRIVER_H
#define ABSTRACTEDFDRIVER_H

#include <QString>
#include <QStringList>
#include <QVector>

class AbstractEdfDriver
{
public:
    struct OperatorInfo
    {
        QString id;
        QString prefix;
        QString name;
    };

    struct CounterpartyInfo
    {
        QString id;
        QString globalId;
        QString organizationName;
        QString inn;
        QString kpp;
    };

    struct ContactInfo
    {
        QString id;
        CounterpartyInfo counterparty;
        OperatorInfo operatorInfo;
    };

    struct ContactsPage
    {
        int count = 0;
        QVector<ContactInfo> contacts;
    };

    struct InvitationInfo
    {
        QString id;
        QString invitationStatus;
        CounterpartyInfo recipient;
        OperatorInfo operatorInfo;
    };

    struct InvitationsPage
    {
        int count = 0;
        QVector<InvitationInfo> invitations;
    };

    explicit AbstractEdfDriver() {}
    virtual ~AbstractEdfDriver() = default;

    AbstractEdfDriver(const AbstractEdfDriver &obj) = delete;
    AbstractEdfDriver &operator=(const AbstractEdfDriver &obj) = delete;

    virtual ContactsPage fetchCounterparties(const QString &filter,
                                             int offset,
                                             int limit) = 0;

    virtual InvitationsPage fetchOutgoingInvitations(const QString &filter,
                                                     int offset,
                                                     int limit) = 0;

    virtual bool inviteCounterpartyByInn(const QString &inn,
                                                const QString &kpp) = 0;
};

#endif // ABSTRACTEDFDRIVER_H

