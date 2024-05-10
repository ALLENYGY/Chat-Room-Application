#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

using namespace std;

class Group;
class User;

class User {
private:
    string userName;
public:
    User(string name) : userName(move(name)) {}
    string getUsername() const {
        return userName;
    }
};

class Group {
public:
    User* admin;
    string groupName;
    vector<User*> memberList;
    Group(User* adm, string name) : admin(adm), groupName(move(name))  {
        memberList.push_back(adm);
        cout << "Group " << groupName << " created by " << admin->getUsername() << endl;
    }

    void addMemberToGroup(User* user) {
        if (find(memberList.begin(), memberList.end(), user) == memberList.end()) {
            memberList.push_back(user);
        }
    }

    void deleteMemberFromGroup(User* user) {
        auto it = find(memberList.begin(), memberList.end(), user);
        if (it != memberList.end()) {
            memberList.erase(it);
        }
    }

    void printMembers() const {
        for (auto member : memberList) {
            cout << "Member Name: " << member->getUsername() << endl;
        }
    }

    User* getAdmin() const {
        return admin;
    }

    // Check if the user is a member of the group
    bool isMember(User* user) {
        return find(memberList.begin(), memberList.end(), user) != memberList.end();
    }

};
