#ifndef ASTRALEDF_H
#define ASTRALEDF_H

#include <string>

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <cpr/cpr.h>

#include "abstractedfdriver.h"

class AstralEdf : public AbstractEdfDriver
{
public:
    static const std::string kBaseUrl;

    static AstralEdf &globalApi(const std::string &accessToken = std::string(),
                                const std::string &abonentId = std::string())
    {
        static AstralEdf api(accessToken, abonentId);
        return api;
    }

    // Downloads list of counterparties for current abonent.
    ContactsPage fetchCounterparties(const QString &filter,
                                     int offset,
                                     int limit) override
    {
        ContactsPage result;

        std::string url = kBaseUrl + "async/v1/Counterparties/contacts";
        std::string query;
        if (!filter.isEmpty()) query += "filter=" + filter.toStdString();
        if (offset > 0) {
            if (!query.empty()) query += "&";
            query += "offset=" + std::to_string(offset);
        }
        if (limit > 0) {
            if (!query.empty()) query += "&";
            query += "limit=" + std::to_string(limit);
        }
        if (!query.empty()) url += "?" + query;

        m_session.SetOption(cpr::Url{url});
        cpr::Response r = m_session.Get();
        if (r.status_code != 200) {
            return result;
        }

        QJsonDocument jsonResponse = QJsonDocument::fromJson(QByteArray::fromStdString(r.text));
        const QJsonObject root = jsonResponse.object();
        result.count = root.value("count").toInt();
        const QJsonArray dataArray = root.value("data").toArray();

        result.contacts.reserve(dataArray.size());
        for (const QJsonValue &value : dataArray) {
            const QJsonObject contactObject = value.toObject();

            ContactInfo contactInfo;
            contactInfo.id = contactObject.value("id").toString();

            const QJsonObject counterpartyObject = contactObject.value("counterparty").toObject();
            fillCounterpartyFromJson(counterpartyObject, contactInfo.counterparty);

            const QJsonObject operatorObject = contactObject.value("operator").toObject();
            if (!operatorObject.isEmpty()) {
                contactInfo.operatorInfo.id = operatorObject.value("id").toString();
                contactInfo.operatorInfo.prefix = operatorObject.value("prefix").toString();
                contactInfo.operatorInfo.name = operatorObject.value("name").toString();
            }

            result.contacts.append(contactInfo);
        }

        return result;
    }

    InvitationsPage fetchOutgoingInvitations(const QString &filter,
                                             int offset,
                                             int limit) override
    {
        InvitationsPage result;

        std::string url = kBaseUrl + "async/v1/counterparties/invitations/outgoing";
        std::string query;
        if (!filter.isEmpty()) query += "filter=" + filter.toStdString();
        if (offset > 0) {
            if (!query.empty()) query += "&";
            query += "offset=" + std::to_string(offset);
        }
        if (limit > 0) {
            if (!query.empty()) query += "&";
            query += "limit=" + std::to_string(limit);
        }
        if (!query.empty()) url += "?" + query;

        m_session.SetOption(cpr::Url{url});
        cpr::Response r = m_session.Get();
        if (r.status_code != 200) {
            return result;
        }

        QJsonDocument jsonResponse = QJsonDocument::fromJson(QByteArray::fromStdString(r.text));
        const QJsonObject root = jsonResponse.object();
        result.count = root.value("count").toInt();
        const QJsonArray dataArray = root.value("data").toArray();

        result.invitations.reserve(dataArray.size());
        for (const QJsonValue &value : dataArray) {
            const QJsonObject invitationObject = value.toObject();

            InvitationInfo invitationInfo;
            invitationInfo.id = invitationObject.value("id").toString();
            invitationInfo.invitationStatus = invitationObject.value("invitationStatus").toString();

            fillCounterpartyFromJson(invitationObject.value("recipient").toObject(), invitationInfo.recipient);

            const QJsonObject operatorObject = invitationObject.value("operator").toObject();
            if (!operatorObject.isEmpty()) {
                invitationInfo.operatorInfo.id = operatorObject.value("id").toString();
                invitationInfo.operatorInfo.prefix = operatorObject.value("prefix").toString();
                invitationInfo.operatorInfo.name = operatorObject.value("name").toString();
            }

            result.invitations.append(invitationInfo);
        }

        return result;
    }

    bool inviteCounterpartyByInn(const QString &inn,
                                 const QString &kpp) override
    {
        const QJsonObject globalIdsObject = fetchGlobalIdsByInn(inn, kpp);
        const QJsonArray dataArray = globalIdsObject.value("data").toArray();

        if (dataArray.isEmpty()) {
            return false;
        }

        bool allSuccess = true;

        for (const QJsonValue &value : dataArray) {
            const QString recipientGlobalId = value.toString();
            if (recipientGlobalId.isEmpty()) {
                allSuccess = false;
                continue;
            }
            const QString processId = sendConnectionInvitation(recipientGlobalId);
            if (processId.isEmpty()) {
                allSuccess = false;
            }
        }
        return allSuccess;
    }

private:
    explicit AstralEdf(const std::string &accessToken,
                       const std::string &abonentId)
        : m_abonentId(abonentId)
    {
        m_header.insert({"Content-Type", "application/json"});
        m_header.insert({"Accept", "application/json"});

        if (!accessToken.empty()) {
            // Adjust header name and format if API requires different authentication scheme.
            m_header.insert({"Authorization", "Bearer " + accessToken});
        }
        if (!m_abonentId.empty()) {
            m_header.insert({"abonentId", m_abonentId});
        }

        m_session.SetOption(m_header);
    }

    QJsonObject fetchGlobalIdsByInn(const QString &inn, const QString &kpp)
    {
        std::string url = kBaseUrl + "async/v1/Counterparties/" + inn.toStdString() + "/globalId";
        if (!kpp.isEmpty()) url += "?kpp=" + kpp.toStdString();

        m_session.SetOption(cpr::Url{url});
        cpr::Response r = m_session.Get();
        if (r.status_code != 200) {
            return QJsonObject();
        }

        QJsonDocument jsonResponse = QJsonDocument::fromJson(QByteArray::fromStdString(r.text));
        return jsonResponse.object();
    }

    QString sendConnectionInvitation(const QString &recipientGlobalId)
    {
        std::string url = kBaseUrl + "async/v1/Counterparties/" + recipientGlobalId.toStdString() + "/connection";

        m_session.SetOption(cpr::Url{url});
        cpr::Response r = m_session.Post();
        if (r.status_code != 200) {
            return QString();
        }

        const QByteArray responseBytes = QByteArray::fromStdString(r.text);
        return QString::fromUtf8(responseBytes).trimmed();
    }

    void fillCounterpartyFromJson(const QJsonObject &object, CounterpartyInfo &info) const
    {
        info.id = object.value("id").toString();
        info.globalId = object.value("globalId").toString();
        info.organizationName = object.value("organizationName").toString();
        info.inn = object.value("inn").toString();
        info.kpp = object.value("kpp").toString();
    }

    std::string m_abonentId;
    cpr::Session m_session;
    cpr::Header m_header;
};

inline const std::string AstralEdf::kBaseUrl = "https://api.doc.astral.ru/";

#endif // ASTRALEDF_H
