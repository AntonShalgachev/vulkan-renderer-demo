#pragma once

class Services;

class ServiceContainer
{
public:
    ServiceContainer(Services& services) : m_services(&services) {}
    Services& services() { assert(m_services); return *m_services; }

private:
    Services* m_services = nullptr;
};
