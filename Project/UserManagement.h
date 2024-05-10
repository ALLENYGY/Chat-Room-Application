// UserManagement.h
#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

class User {
private:
    std::string userID;
    std::string userName;
public:
    User(std::string ID, std::string name) : userID(std::move(ID)), userName(std::move(name)) {}
    std::string getUsername() const { return userName; }
};

class Group {
public:
    User* admin;
    std::string groupName;
    std::vector<User*> memberList;

    Group(User* adm, std::string name) : admin(adm), groupName(std::move(name)) {
        memberList.push_back(adm);
        std::cout << "Group " << groupName << " created by " << admin->getUsername() << std::endl;
    }

    void addMemberToGroup(User* user) {
        if (std::find(memberList.begin(), memberList.end(), user) == memberList.end()) {
            memberList.push_back(user);
        }
    }

    void deleteMemberFromGroup(User* user) {
        auto it = std::find(memberList.begin(), memberList.end(), user);
        if (it != memberList.end()) {
            memberList.erase(it);
        }
    }

    void printMembers() const {
        for (auto member : memberList) {
            std::cout << "Member Name: " << member->getUsername() << std::endl;
        }
    }

    User* getAdmin() const {
        return admin;
    }

    bool isMember(User* user) const {
        return std::find(memberList.begin(), memberList.end(), user) != memberList.end();
    }
};

#endif // USER_MANAGEMENT_H
