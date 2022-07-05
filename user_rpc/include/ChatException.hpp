#pragma once
#include <exception>
#include "public.hpp"

class ChatException {
public:
    ChatException(int ack, std::exception& e)
        : m_ackNum(ack), m_exception(e) {}

    int getAck() { return m_ackNum; }
    std::exception getException() { return m_exception; }

    void setAck(int ack) { m_ackNum = ack; }
    void setException(std::exception e) { m_exception = e; }
private:
    int m_ackNum;
    std::exception m_exception;
};