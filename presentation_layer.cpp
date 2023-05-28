#include "presentation_layer.h"
#include "business_layer_records.h"
#include "business_layer_tables.h"
#include "business_layer_user.h"
#include "data_layer_records.h"
#include "data_layer_tables.h"
#include "data_layer_users.h"
#include "useragreement.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

using namespace std;

string Universal_password = "NULL";
int Universal_TableID = 0;
string Universal_TableName = "";

// CHATGPT HELP
void setStdinEcho(bool enable) {
  termios tty;
  tcgetattr(STDIN_FILENO, &tty);
  if (!enable)
    tty.c_lflag &= ~ECHO;
  else
    tty.c_lflag |= ECHO;

  tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// CHATGPT HELP
string getPassword() {
  string password;
  char ch;
  setStdinEcho(false);

  // censors the password
  while ((ch = getchar()) != '\n') {
    if (ch != 127 && ch != 8) { // Ignore delete and backspace keys
      password.push_back(ch);
      cout << '*';
    } else if (!password
                    .empty()) { // Remove the last character from the password
      password.pop_back();
      cout << "\b \b"; // Move cursor back, overwrite with space, and move
                       // cursor back again
    }
  }

  setStdinEcho(true);
  cout << endl;
  return password;
}

void loginMenu(User *&currentUser, bool &tableMenuFlag, vector<User> &users) {
  int choice;
  cout << "╔══════════════════════════════════════════════╗" << endl;
  cout << "║                                              ║" << endl;
  cout << "║         Welcome to the NullDB!               ║" << endl;
  cout << "║                                              ║" << endl;
  cout << "╠══════════════════════════════════════════════╣" << endl;
  cout << "║ 1. Log in                                    ║" << endl;
  cout << "║ 2. Create user                               ║" << endl;
  cout << "║ 3. Exit                                      ║" << endl;
  cout << "║                                              ║" << endl;
  cout << "╚══════════════════════════════════════════════╝" << endl;
  cin >> choice;

  string username, password;
  cout << "Enter username: ";
  cin >> username;
  cout << "Enter password: ";
  cin.ignore();
  password = getPassword();
  Universal_password = password;

  switch (choice) {
  case 1: {
    currentUser = login(username, password, users);
    if (!currentUser) {
      cout << "Invalid credentials. Please try again." << endl;
    } else {
      cout << "Welcome, " << currentUser->username << "!" << endl;
      cout << "USER_ID: " << currentUser->id << endl;
      tableMenuFlag = true;
    }
    break;
  }
  case 2: {
    bool isManager = false;
    string managerCode = "CDkey123";
    cout << "Do you want to make the new user a system manager? (yes/no): ";
    string response;
    cin >> response;
    if (response == "yes") {
      cout << "Enter the manager code: ";
      string code;
      cin >> code;
      if (code == managerCode) {
        isManager = true;
      } else {
        cout << "Invalid code. User will not be a manager.\n";
      }
    }
    currentUser = createUser(username, password, isManager, users);
    saveUsers(users);
    Universal_password = password;
    if (!currentUser) {
      cout << "User name is taken. Please try again." << endl;
      tableMenuFlag = false;
    } else {
      cout << "User created successfully!" << endl;
      cout << "USER_ID: " << currentUser->id << endl;
      tableMenuFlag = true;
    }
    break;
  }
  case 3: {
    cout << "Exiting NullDB. Goodbye!" << endl;
    if (logout(currentUser)) {
      break;
    }
    return;
  }
  default: {
    cout << "Invalid choice! Please try again." << endl;
    break;
  }
  }
  return;
}

void tableMenu(User *currentUser, bool &tableMenuFlag, vector<Table> &tables,
               Table *currentTable, vector<User> &users) {
  int choice;
  cout << "╔══════════════════════════════════════════════╗" << endl;
  cout << "║                                              ║" << endl;
  cout << "║                  Table Menu                  ║" << endl;
  cout << "║                                              ║" << endl;
  cout << "╠══════════════════════════════════════════════╣" << endl;
  cout << "║ Welcome, " << currentUser->username
       << setw(39 - currentUser->username.length()) << " ║" << endl;
  cout << "╠══════════════════════════════════════════════╣" << endl;
  cout << "║ 1. Create a table                            ║" << endl;
  cout << "║ 2. View all tables                           ║" << endl;
  cout << "║ 3. Enter a table                             ║" << endl;
  cout << "║ 4. Delete a table                            ║" << endl;
  cout << "║ 5. Add a collaborator to a table             ║" << endl;
  cout << "║ 6. Exit                                      ║" << endl;
  cout << "║                                              ║" << endl;
  cout << "╚══════════════════════════════════════════════╝" << endl;
  cin >> choice;

  switch (choice) {
  case 1: {
    string name = "";
    cin.ignore();
    cout << "Enter a name for your table: ";
    getline(cin, name);
    int tableID = createNewTable(currentUser->username, name, tables);
    cout << "Created new table with id: " << tableID << endl;
    // Save tables after a new table has been created
    saveTables(tables);
    tableMenuFlag = false;
    Universal_TableID = tableID;
    Universal_TableName = name;
    break;
  } // end case 1
  case 2:
    // Load tables before printing
    tables = loadTables();
    // printTables(currentUser);
    printTableDetails(tables, users);
    tableMenuFlag = true;
    break;
  case 3: {
    int tableID = 0;
    cout << "Enter the ID of table to go into: " << endl;
    cin >> tableID;
    cin.ignore();
    currentTable = loadExistingTable(tableID, tables);
    if (!currentTable) {
      cerr << "Oops! That table doesn't exist. Try again!" << endl;
    } else {
      // Check if the current user is the owner or an authorized collaborator
      if (currentTable->owner == currentUser->username ||
          find(currentTable->authorizedCollaborators.begin(),
               currentTable->authorizedCollaborators.end(), currentUser->id) !=
              currentTable->authorizedCollaborators.end()) {
        cout << "Loading " << currentTable->name << "..." << endl;
        Universal_TableID = currentTable->id;
        Universal_TableName = currentTable->name;
      } else {
        tableMenuFlag = true;
        cout << "You do not have the rights to access this table." << endl;
      }
    }
    tableMenuFlag = false;
    break;
  }
  case 4:
    // Load tables before deletion
    tables = loadTables();
    // TODO: Implement delete function
    // After deletion, save tables
    saveTables(tables);
    tableMenuFlag = false;
    break;
  case 5: {
    int tableID = 0;
    int userID = 0;
    cout << "Enter the ID of the table to add a collaborator to: ";
    cin >> tableID;
    cin.ignore();
    cout << "Enter the ID of the user to be added as a collaborator: ";
    cin >> userID;
    cin.ignore();
    // Add a collaborator to a table
    addCollaborator(tableID, userID, tables);
    // Save tables after adding a collaborator
    saveTables(tables);
    tableMenuFlag = false;
    break;
  }
  case 6:
    cout << "Exiting NullDB. Goodbye!" << endl;
    tableMenuFlag = false;
    if (logout(currentUser)) {
      break; // break out of the inner while-loop if logout was successful
    }
    return;
  default:
    cout << "Invalid choice! Please try again." << endl;
  }
} // end TABLE

void recordMenu(User *currentUser, bool tableMenuFlag, vector<Table> &tables,
                Table *currentTable, vector<Record> &records,
                vector<User> &users) {
  int choice;
  cout << "╔══════════════════════════════════════════════╗" << endl;
  cout << "║                                              ║" << endl;
  cout << "║                 Record Menu                  ║" << endl;
  cout << "║                                              ║" << endl;
  cout << "╠══════════════════════════════════════════════╣" << endl;
  cout << "║ 0. Display by ID                             ║" << endl;
  cout << "║ 1. Insert                                    ║" << endl;
  cout << "║ 2. Delete                                    ║" << endl;
  cout << "║ 3. Update                                    ║" << endl;
  cout << "║ 4. Display Records                           ║" << endl;
  cout << "║ 5. Keyword Search                            ║" << endl;
  cout << "║ 6. Term Frequency                            ║" << endl;
  cout << "║ 7. Sorting (Sub-Menu)                        ║" << endl;
  cout << "║ 8. Save and Export                           ║" << endl;
  cout << "║ 9. Save and Exit (Back to Table Menu)        ║" << endl;
  cout << "║ 10. Log out                                  ║" << endl;
  cout << "║                                              ║" << endl;
  cout << "╚══════════════════════════════════════════════╝" << endl;

  cin >> choice;

  switch (choice) {
  case 0: {
    int id;
    string password;
    cout << "Enter record ID to display: ";
    cin >> id;
    cin.ignore();

    Record record = getRecordById(id, records);
    if (record.encryptionType != "NONE") {
      cout << "This record is protected. Please enter the password: ";
      getline(cin, password);
    } else {
      password = "NONE";
    }

    auto decryptedRecord = displayRecord(id, records, currentUser, password);
    if (!decryptedRecord.empty()) {
      cout << "Record ID: " << decryptedRecord[0].id
           << "\nData: " << decryptedRecord[0].data << endl;
    } else {
      cout << "Either the record does not exist, you do not have "
              "permission to view it, or the password is incorrect."
           << endl;
    }
    break;
  }

  case 1: {
    string data;
    string temp;
    string en_key;
    cin.ignore();
    cout << "Enter data: ";
    getline(cin, data);
    cout << "Do you want to encrypt this data? (Y/N)" << endl
         << "Warning: using encryption would not allow you to do a "
            "search on this record"
         << endl;
    getline(cin, temp);
    if (temp == "Y") {
      cout << "Please enter a 1-6 leghth password for this particular "
              "data. (PLEASE REMEMBER THIS KEY)"
           << endl;
      getline(cin, en_key);
      int id = insert(data, currentUser->username, records, 1, en_key,
                      Universal_TableID, Universal_TableName);
      //
      cout << "Inserted record with id: " << id << endl;
    } else {
      int id = insert(data, currentUser->username, records, 0, "NONE",
                      Universal_TableID, Universal_TableName);
      cout << "Inserted record with id: " << id << endl;
    }

    break;
  }
  case 2: {
    int id;
    string password;
    cout << "Enter id to delete: ";
    cin >> id;
    cin.ignore();

    Record record = getRecordById(id, records);
    if (record.encryptionType != "NONE") {
      cout << "This record is protected. Please enter the password: ";
      getline(cin, password);
    } else {
      password = "NONE";
    }

    deleteRecord(id, records, password);
    break;
  }
  case 3: {
    int id;
    string newData;
    string password;
    cout << "Enter id to update: ";
    cin >> id;
    cin.ignore();

    Record record = getRecordById(id, records);
    if (record.encryptionType != "NONE") {
      cout << "This record is protected. Please enter the password: ";
      getline(cin, password);
    } else {
      password = "NONE";
    }

    cout << "Enter new data: ";
    getline(cin, newData);
    update(id, newData, records, password);
    break;
  }
  case 4: {
    for (const auto &record : filterByTableID(
             records, Universal_TableID)) { // Using new function, this
                                            // returns all in the same I
      updateLastRead(record.id, records);
      cout << record.id << ": " << record.data
           << " (Created at: " << record.timestamp
           << ", Last modified: " << record.last_modified << ", Last read: ";
      if (record.last_read.empty()) {
        cout << "Never been read";
      } else {
        cout << record.last_read;
      }
      cout << ")" << endl;
    }
    break;
  }
  case 5: {
    string keyword;
    cout << "Enter keyword: ";
    cin >> keyword;
    for (const auto &record : filterByKeyword(
             filterByCreator(records, currentUser->username), keyword)) {
      cout << record.id << ": " << record.data
           << " (Created at: " << record.timestamp << ")" << endl;
    }
    break;
  }
  case 6: {
    string term;
    cout << "Enter term: ";
    cin >> term;
    int totalFrequency = 0;
    for (const auto &record : filterByCreator(records, currentUser->username)) {
      if (record.encryptionType != "NONE") {
        continue;
      }
      int frequency = countTermFrequency(record.data, term);
      totalFrequency += frequency;
      if (frequency > 0) {
        cout << "Record ID " << record.id << ": " << frequency << " occurrences"
             << endl;
      }
    }
    cout << "Total occurrences of term \"" << term << "\": " << totalFrequency
         << endl;
    break;
  }
  case 7: {
    {
      int sortChoice;
      cout << "Sort Menu\n1. Sort Records (A-Z, Z-A)\n2. Sort Records by "
              "ID (Min to Max, Max to Min)\nEnter your choice: ";
      cin >> sortChoice;

      switch (sortChoice) {
      case 1: {
        int order;
        cout << "Sort Records\n1. A-Z\n2. Z-A\nEnter your choice: ";
        cin >> order;
        sortRecords(records, order == 2);
        cout << "Records sorted." << endl;
        break;
      }
      case 2: {
        int order;
        cout << "Sort Records by ID\n1. Min to Max\n2. Max to Min\nEnter "
                "your choice: ";
        cin >> order;
        sortRecordsById(records, order == 2);
        cout << "Records sorted by ID." << endl;
        break;
      }
      default: {
        cout << "Invalid choice!" << endl;
        break;
      }
      }
      break;
    }
  }

  case 9: {
    saveRecords(records);
    break;
  }
  case 10: {
    saveRecords(records);
    tableMenuFlag = false;
    string tempUser = currentUser->username; // Saves current user name for exit message
    if (logout(currentUser)) {
      currentUser = nullptr; // Set currentUser to nullptr to indicate logout
      cout << "Goodbye, " << tempUser << endl;
      break; // break out of the inner while-loop if logout was successful
    }
    cout << "Log out was unsuccessful" << endl;
    break;
  }

  default: {
    cout << "Invalid choice!" << endl;
    break;
  }
  }
}

int main() {
  vector<Table> tables = loadTables();
  vector<Record> records = loadRecords();
  vector<User> users = loadUsers();

  if (!isManager("123", users)) {
    createUser("123", "123", false, users);
    createUser("321", "321", false, users);
    saveUsers(users);
  }

  User *currentUser = nullptr;
  Table *currentTable = nullptr;
  bool tableMenuFlag = false;
  bool userAgreement = getUserAgreement();

  while (userAgreement) {
    while (!currentUser) {
      loginMenu(currentUser, tableMenuFlag, users);
    }

    while (currentUser) {
      tableMenu(currentUser, tableMenuFlag, tables, currentTable, users);

      while (!tableMenuFlag) {
        recordMenu(currentUser, tableMenuFlag, tables, currentTable, records,
                   users);
      }
    }
  }

  return 0;
}
