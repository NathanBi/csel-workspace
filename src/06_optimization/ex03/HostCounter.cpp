#include "HostCounter.h"

#include <algorithm> // for std::find
#include <unordered_map>

int cpt = 0;
std::unordered_map<std::string, int> map;

HostCounter::HostCounter()
{

}

bool HostCounter::isNewHost(std::string hostname)
{

    //return std::find(myHosts.begin(), myHosts.end(), hostname) == myHosts.end();
    if(map.find(hostname) == map.end())
    {
        map[hostname] = cpt++;
        return true;
    }
    else
    {
        return false;
    }

    
}

void HostCounter::notifyHost(std::string hostname)
{
    // add the host in the list if not already in
    if(isNewHost(hostname))
    {
        myHosts.push_back(hostname);
    }
}

int HostCounter::getNbOfHosts()
{
    return myHosts.size();
}
