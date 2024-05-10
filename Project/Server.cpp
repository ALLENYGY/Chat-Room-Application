#include "UserManagement.h"

using namespace std;

class Server {
private:
    map<string, Group*> groups;
    map<string, User*> users;

public:
    void addUser(string userID,string userName) {
        // Check if the user already exists
        if (users.find(userName) != users.end()) {
            cout << "User " << userName << " already exists" << endl;
            return;
        }
        users[userName] = new User(userID,userName);
    }

    void createGroup(string userName, string groupName) {
        // Check if the user exists and group does not exist
        if (users.find(userName) != users.end() && groups.find(groupName) == groups.end()) {
            groups[groupName] = new Group(users[userName], groupName); 
        }else{
            cout << "Group " << groupName << " already exists" << endl;
        }
    }

    void addMemberToGroup(string userName, string groupName, string memberName) {
        // Check if the user, group and member already exist
        if (users.find(userName) != users.end() 
                && groups.find(groupName) != groups.end() 
                && users.find(memberName) != users.end()
                && groups[groupName]->getAdmin() == users[userName]){
            // check if the member is already in the group
            if(groups[groupName]->isMember(users[memberName])) {
                cout << "User " << memberName << " is already a member of the group " << groupName << endl;
                return;
            }
            // Add member to the group
            groups[groupName]->addMemberToGroup(users[memberName]);
            cout << "User " << memberName << " has been added to the group " << groupName << endl;
        }
    }

    void deleteMemberFromGroup(string userName, string groupName, string memberName) {
        // Check if the user, group and member already exist
        if (users.find(userName) != users.end() && groups.find(groupName) != groups.end() && users.find(memberName) != users.end()) {
            // Check if the user is the admin of the group
            if (groups[groupName]->getAdmin() == users[userName]) {
                cout << "You cannot quit the group("<<groupName<<") you created"<< endl;
                return;
            }
            // Check if the member is in the group
            if (!groups[groupName]->isMember(users[memberName])) {
                cout << "User " << memberName << " is not a member of the group " << groupName << endl;
                return;
            }
            groups[groupName]->deleteMemberFromGroup(users[memberName]);
            cout << "User " << memberName << " has been deleted from the group " << groupName << endl;
        }
    }

    void quitGroup(string userName, string groupName) {
        // Check if the user and group already exist
        if (users.find(userName) != users.end() && groups.find(groupName) != groups.end()) {
            // Check if the user is the admin of the group
            if (groups[groupName]->getAdmin() == users[userName]) {
                cout << "You cannot quit the group("<<groupName<<") you created"<< endl;
                return;
            }
            // Check if the user is in the group
            if (!groups[groupName]->isMember(users[userName])) {
                cout << "You are not a member of the group " << groupName << endl;
                return;
            }
            groups[groupName]->deleteMemberFromGroup(users[userName]);
            cout << "User " << userName << " has quit the group " << groupName << " Successfully! " <<endl;
        }   
    }

    // More methods as necessary...
};


int main() {
    Server server;

    // Test Case 1: Adding Users
    cout << "Test Case 1: Adding Users" << endl;
    server.addUser("1","Alice");
    server.addUser("2","Bob");
    server.addUser("1","Alice"); // Should show an error that user already exists

    // Test Case 2: Creating Groups
    cout << "\nTest Case 2: Creating Groups" << endl;
    server.createGroup("Alice", "Coders");
    server.createGroup("Charlie", "Coders"); // Error: Charlie does not exist
    server.createGroup("Alice", "Coders"); // Error: Group already exists

    // Test Case 3: Adding Members to Groups
    cout << "\nTest Case 3: Adding Members to Groups" << endl;
    server.addMemberToGroup("Alice", "Coders", "Bob");
    server.addMemberToGroup("Alice", "Coders", "Bob"); // Bob is already a member
    server.addMemberToGroup("Alice", "Gamers", "Bob"); // Group does not exist
    server.addMemberToGroup("Charlie", "Coders", "Bob"); // Charlie is not an admin

    // Test Case 4: Deleting Members from Groups
    cout << "\nTest Case 4: Deleting Members from Groups" << endl;
    server.deleteMemberFromGroup("Alice", "Coders", "Bob");
    server.deleteMemberFromGroup("Alice", "Coders", "Charlie"); // Charlie is not a member

    // Test Case 5: Users Quitting Groups
    cout << "\nTest Case 5: Users Quitting Groups" << endl;
    server.quitGroup("Alice", "Coders"); // Admin cannot quit
    server.quitGroup("Bob", "Coders"); // Not a member anymore
    server.quitGroup("Charlie", "Coders"); // Not a member

    return 0;
}
