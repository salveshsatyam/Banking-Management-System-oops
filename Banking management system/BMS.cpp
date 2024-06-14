
#include <bits/stdc++.h>
#include <mysql.h>
#include <mysqld_error.h>

using namespace std;

const char* HOST = "localhost";
const char* USER = "root";
const char* PW = "Salvesh@123#";
const char* DB = "testdb";

// Convert integer to string
string to_string(int number) {
    ostringstream oss;
    oss << number;
    return oss.str();
}

// Convert double to string
string to_string(double number) {
    ostringstream oss;
    oss << number;
    return oss.str();
}

// Convert string to double
double stod(const string& str) {
    istringstream iss(str);
    double val;
    iss >> val;
    return val;
}

class BankAccount {
public:
    string name;
    int accountNumber;
    double balance;

    BankAccount() {
        this->name = "";
        this->accountNumber = 0;
        this->balance = 0.0;
    }

    BankAccount(string n, int a, double b) {
        this->name = n;
        this->accountNumber = a;
        this->balance = b;
    }

    virtual void withdraw(double amount) = 0;

    void deposit(double amount) {
        balance += amount;
    }

    void display() {
        cout << "Name: " << name << endl;
        cout << "Account Number: " << accountNumber << endl;
        cout << "Balance: " << balance << endl;
    }
};

class SavingsAccount : public BankAccount {
public:
    static const double MAX_WITHDRAWAL_LIMIT;

public:
    SavingsAccount() : BankAccount() {}

    SavingsAccount(string n, int a, double b) : BankAccount(n, a, b) {}

    void withdraw(double amount) override {
        double minimumBalance = 2000.0;

        if (balance - amount >= minimumBalance && amount <= MAX_WITHDRAWAL_LIMIT) {
            balance -= amount;
        } else {
            cout << "Invalid withdrawal amount or insufficient funds for withdrawal from SavingsAccount." << endl;
        }
    }
};

const double SavingsAccount::MAX_WITHDRAWAL_LIMIT = 20000.0;

class CurrentAccount : public BankAccount {
public:
    CurrentAccount() : BankAccount() {}

    CurrentAccount(string n, int a, double b) : BankAccount(n, a, b) {}

    void withdraw(double amount) override {
        double minimumBalance = 10000.0;
        if (balance - amount >= minimumBalance) {
            balance -= amount;
        } else {
            cout << "Insufficient funds for withdrawal from CurrentAccount." << endl;
        }
    }
};

void createAccount(MYSQL* con) {
    string name;
    int accountNumber;
    double balance;

    cout << "Enter the name for the account: ";
    cin >> name;

    cout << "Enter the account number for the account: ";
    cin >> accountNumber;

    cout << "Enter the initial balance for the account: ";
    cin >> balance;

    cout << "Enter account type (1 for Savings, 2 for Current): ";
    int accountType;
    cin >> accountType;

    string accountTypeName = (accountType == 1) ? "Savings" : "Current";

    string query = "INSERT INTO accounts (name, accountNumber, balance, accountType) VALUES ('" +
                    name + "', " + to_string(accountNumber) + ", " + to_string(balance) + ", '" + accountTypeName + "')";

    if (mysql_query(con, query.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(con));
    }
}

void deposit(MYSQL* con) {
    int accountNumber;
    double depositAmount;

    cout << "Enter the account number to deposit into: ";
    cin >> accountNumber;
    cout << "Enter the amount to deposit: ";
    cin >> depositAmount;

    string query = "UPDATE accounts SET balance = balance + " + to_string(depositAmount) +
                   " WHERE accountNumber = " + to_string(accountNumber);

    if (mysql_query(con, query.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(con));
    }
}

void withdraw(MYSQL* con) {
    int accountNumber;
    double withdrawAmount;

    cout << "Enter the account number to withdraw from: ";
    cin >> accountNumber;
    cout << "Enter the amount to withdraw: ";
    cin >> withdrawAmount;

    string query = "SELECT balance, accountType FROM accounts WHERE accountNumber = " + to_string(accountNumber);
    if (mysql_query(con, query.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return;
    }

    MYSQL_RES* res = mysql_store_result(con);
    if (res == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        double balance = stod(row[0]);
        string accountType = row[1];
        bool canWithdraw = false;

        if (accountType == "Savings") {
            double minimumBalance = 2000.0;
            if (balance - withdrawAmount >= minimumBalance && withdrawAmount <= SavingsAccount::MAX_WITHDRAWAL_LIMIT) {
                canWithdraw = true;
            }
        } else if (accountType == "Current") {
            double minimumBalance = 10000.0;
            if (balance - withdrawAmount >= minimumBalance) {
                canWithdraw = true;
            }
        }

        if (canWithdraw) {
            string updateQuery = "UPDATE accounts SET balance = balance - " + to_string(withdrawAmount) +
                                 " WHERE accountNumber = " + to_string(accountNumber);
            if (mysql_query(con, updateQuery.c_str())) {
                fprintf(stderr, "%s\n", mysql_error(con));
            }
        } else {
            cout << "Invalid withdrawal amount or insufficient funds." << endl;
        }
    } else {
        cout << "Account not found." << endl;
    }

    mysql_free_result(res);
}

void display(MYSQL* con) {
    int accountNumber;
    cout << "Enter account number to see details: ";
    cin >> accountNumber;

    string query = "SELECT name, accountNumber, balance FROM accounts WHERE accountNumber = " + to_string(accountNumber);
    if (mysql_query(con, query.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return;
    }

    MYSQL_RES* res = mysql_store_result(con);
    if (res == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        cout << "Name: " << row[0] << endl;
        cout << "Account Number: " << row[1] << endl;
        cout << "Balance: " << row[2] << endl;
    } else {
        cout << "Account not found." << endl;
    }

    mysql_free_result(res);
}

void transfer(MYSQL* con) {
    int senderAccount, receiverAccount;
    double transferAmount;

    cout << "Enter sender's account number: ";
    cin >> senderAccount;

    cout << "Enter receiver's account number: ";
    cin >> receiverAccount;

    cout << "Enter amount to transfer: ";
    cin >> transferAmount;

    string senderQuery = "SELECT balance FROM accounts WHERE accountNumber = " + to_string(senderAccount);
    if (mysql_query(con, senderQuery.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return;
    }

    MYSQL_RES* senderRes = mysql_store_result(con);
    if (senderRes == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return;
    }

    MYSQL_ROW senderRow = mysql_fetch_row(senderRes);
    if (!senderRow) {
        cout << "Sender account not found." << endl;
        mysql_free_result(senderRes);
        return;
    }
    double senderBalance = stod(senderRow[0]);

    string receiverQuery = "SELECT balance FROM accounts WHERE accountNumber = " + to_string(receiverAccount);
    if (mysql_query(con, receiverQuery.c_str())) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_free_result(senderRes);
        return;
    }

    MYSQL_RES* receiverRes = mysql_store_result(con);
    if (receiverRes == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_free_result(senderRes);
        return;
    }

    MYSQL_ROW receiverRow = mysql_fetch_row(receiverRes);
    if (!receiverRow) {
        cout << "Receiver account not found." << endl;
        mysql_free_result(senderRes);
        mysql_free_result(receiverRes);
        return;
    }

    if (senderBalance >= transferAmount) {
        string updateSender = "UPDATE accounts SET balance = balance - " + to_string(transferAmount) +
                              " WHERE accountNumber = " + to_string(senderAccount);
        if (mysql_query(con, updateSender.c_str())) {
            fprintf(stderr, "%s\n", mysql_error(con));
        }

        string updateReceiver = "UPDATE accounts SET balance = balance + " + to_string(transferAmount) +
                                " WHERE accountNumber = " + to_string(receiverAccount);
        if (mysql_query(con, updateReceiver.c_str())) {
            fprintf(stderr, "%s\n", mysql_error(con));
        }
    } else {
        cout << "Insufficient funds!" << endl;
    }

    mysql_free_result(senderRes);
    mysql_free_result(receiverRes);
}

int main() {
    MYSQL* con = mysql_init(NULL);

    if (!mysql_real_connect(con, HOST, USER, PW, DB, 3306, NULL, 0)) {
        cout << "Error: " << mysql_error(con) << endl;
        return 1;
    } else {
        cout << "Logged IN!" << endl;
    }

    int choice = 0;
    while (choice < 6) {
        cout << "Welcome to BHARAT BANK!" << endl;
        cout << endl;
        cout << "0. Create" << endl;
        cout << "1. Deposit" << endl;
        cout << "2. Withdraw" << endl;
        cout << "3. Display" << endl;
        cout << "4. Transfer" << endl;
        cout << "5. Exit" << endl;

        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 0:
                createAccount(con);
                break;
            case 1:
                deposit(con);
                break;
            case 2:
                withdraw(con);
                break;
            case 3:
                display(con);
                break;
            case 4:
                transfer(con);
                break;
            case 5:
                break;
            default:
                cout << "Invalid choice." << endl;
                break;
        }
    }

    mysql_close(con);
    return 0;
}

