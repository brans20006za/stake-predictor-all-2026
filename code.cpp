#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

using namespace std;

/*
    ================================
    Simple Library Management System
    ================================
    Features:
    - Add books
    - Remove books
    - Search books
    - Borrow / Return books
    - Save / Load data
    - User management
    - Statistics
*/

// Utility functions
namespace Utils
{
    string toLower(const string& s)
    {
        string r = s;
        transform(r.begin(), r.end(), r.begin(), ::tolower);
        return r;
    }

    string getTimeStamp()
    {
        time_t now = time(nullptr);
        tm* ltm = localtime(&now);

        stringstream ss;
        ss << 1900 + ltm->tm_year << "-"
           << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
           << setw(2) << setfill('0') << ltm->tm_mday << " "
           << setw(2) << setfill('0') << ltm->tm_hour << ":"
           << setw(2) << setfill('0') << ltm->tm_min;

        return ss.str();
    }

    void pause()
    {
        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
    }
}

// Book class
class Book
{
private:
    int id;
    string title;
    string author;
    int year;
    bool borrowed;

public:
    Book()
        : id(0), year(0), borrowed(false) {}

    Book(int i, string t, string a, int y)
        : id(i), title(t), author(a), year(y), borrowed(false) {}

    int getId() const { return id; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    int getYear() const { return year; }
    bool isBorrowed() const { return borrowed; }

    void setBorrowed(bool b) { borrowed = b; }

    string serialize() const
    {
        stringstream ss;
        ss << id << "|"
           << title << "|"
           << author << "|"
           << year << "|"
           << borrowed;

        return ss.str();
    }

    static Book deserialize(const string& line)
    {
        stringstream ss(line);
        string token;

        int id, year;
        bool borrowed;
        string title, author;

        getline(ss, token, '|');
        id = stoi(token);

        getline(ss, title, '|');
        getline(ss, author, '|');

        getline(ss, token, '|');
        year = stoi(token);

        getline(ss, token, '|');
        borrowed = stoi(token);

        Book b(id, title, author, year);
        b.setBorrowed(borrowed);

        return b;
    }

    void print() const
    {
        cout << left
             << setw(5) << id
             << setw(25) << title.substr(0, 23)
             << setw(20) << author.substr(0, 18)
             << setw(8) << year
             << setw(10) << (borrowed ? "Yes" : "No")
             << endl;
    }
};

// User class
class User
{
private:
    int id;
    string name;
    vector<int> borrowedBooks;

public:
    User() : id(0) {}

    User(int i, string n) : id(i), name(n) {}

    int getId() const { return id; }
    string getName() const { return name; }

    void borrowBook(int bookId)
    {
        borrowedBooks.push_back(bookId);
    }

    void returnBook(int bookId)
    {
        borrowedBooks.erase(
            remove(borrowedBooks.begin(), borrowedBooks.end(), bookId),
            borrowedBooks.end()
        );
    }

    bool hasBook(int bookId) const
    {
        return find(borrowedBooks.begin(),
                    borrowedBooks.end(),
                    bookId) != borrowedBooks.end();
    }

    string serialize() const
    {
        stringstream ss;
        ss << id << "|" << name << "|";

        for (size_t i = 0; i < borrowedBooks.size(); i++)
        {
            ss << borrowedBooks[i];
            if (i + 1 < borrowedBooks.size())
                ss << ",";
        }

        return ss.str();
    }

    static User deserialize(const string& line)
    {
        stringstream ss(line);
        string token;

        int id;
        string name;

        getline(ss, token, '|');
        id = stoi(token);

        getline(ss, name, '|');

        User u(id, name);

        getline(ss, token, '|');
        stringstream bs(token);

        string num;
        while (getline(bs, num, ','))
        {
            if (!num.empty())
                u.borrowBook(stoi(num));
        }

        return u;
    }

    void print() const
    {
        cout << "ID: " << id
             << " | Name: " << name
             << " | Borrowed: " << borrowedBooks.size()
             << endl;
    }
};

// Logger
class Logger
{
private:
    ofstream file;

public:
    Logger(const string& name)
    {
        file.open(name, ios::app);
    }

    ~Logger()
    {
        if (file.is_open())
            file.close();
    }

    void log(const string& msg)
    {
        if (!file.is_open())
            return;

        file << "[" << Utils::getTimeStamp() << "] "
             << msg << endl;
    }
};

// Library class
class Library
{
private:
    vector<Book> books;
    vector<User> users;

    Logger logger;

    int nextBookId;
    int nextUserId;

public:
    Library()
        : logger("library.log"),
          nextBookId(1),
          nextUserId(1)
    {
        load();
    }

    ~Library()
    {
        save();
    }

    void addBook()
    {
        string title, author;
        int year;

        cin.ignore();

        cout << "Title: ";
        getline(cin, title);

        cout << "Author: ";
        getline(cin, author);

        cout << "Year: ";
        cin >> year;

        Book b(nextBookId++, title, author, year);
        books.push_back(b);

        logger.log("Added book: " + title);
        cout << "Book added.\n";
    }

    void removeBook()
    {
        int id;
        cout << "Book ID: ";
        cin >> id;

        auto it = remove_if(books.begin(), books.end(),
            [id](const Book& b) {
                return b.getId() == id;
            });

        if (it != books.end())
        {
            books.erase(it, books.end());
            logger.log("Removed book ID: " + to_string(id));
            cout << "Book removed.\n";
        }
        else
        {
            cout << "Not found.\n";
        }
    }

    void listBooks() const
    {
        cout << left
             << setw(5) << "ID"
             << setw(25) << "Title"
             << setw(20) << "Author"
             << setw(8) << "Year"
             << setw(10) << "Borrowed"
             << endl;

        cout << string(70, '-') << endl;

        for (const auto& b : books)
            b.print();
    }

    void searchBooks()
    {
        cin.ignore();

        string query;
        cout << "Search: ";
        getline(cin, query);

        query = Utils::toLower(query);

        bool found = false;

        for (const auto& b : books)
        {
            string t = Utils::toLower(b.getTitle());
            string a = Utils::toLower(b.getAuthor());

            if (t.find(query) != string::npos ||
                a.find(query) != string::npos)
            {
                b.print();
                found = true;
            }
        }

        if (!found)
            cout << "No results.\n";
    }

    void addUser()
    {
        cin.ignore();

        string name;
        cout << "Name: ";
        getline(cin, name);

        User u(nextUserId++, name);
        users.push_back(u);

        logger.log("Added user: " + name);
        cout << "User added.\n";
    }

    User* findUser(int id)
    {
        for (auto& u : users)
        {
            if (u.getId() == id)
                return &u;
        }

        return nullptr;
    }

    Book* findBook(int id)
    {
        for (auto& b : books)
        {
            if (b.getId() == id)
                return &b;
        }

        return nullptr;
    }

    void borrowBook()
    {
        int userId, bookId;

        cout << "User ID: ";
        cin >> userId;

        cout << "Book ID: ";
        cin >> bookId;

        User* u = findUser(userId);
        Book* b = findBook(bookId);

        if (!u || !b)
        {
            cout << "Invalid ID.\n";
            return;
        }

        if (b->isBorrowed())
        {
            cout << "Already borrowed.\n";
            return;
        }

        b->setBorrowed(true);
        u->borrowBook(bookId);

        logger.log("User " + to_string(userId) +
                   " borrowed book " +
                   to_string(bookId));

        cout << "Success.\n";
    }

    void returnBook()
    {
        int userId, bookId;

        cout << "User ID: ";
        cin >> userId;

        cout << "Book ID: ";
        cin >> bookId;

        User* u = findUser(userId);
        Book* b = findBook(bookId);

        if (!u || !b)
        {
            cout << "Invalid ID.\n";
            return;
        }

        if (!u->hasBook(bookId))
        {
            cout << "User does not have this book.\n";
            return;
        }

        b->setBorrowed(false);
        u->returnBook(bookId);

        logger.log("User " + to_string(userId) +
                   " returned book " +
                   to_string(bookId));

        cout << "Returned.\n";
    }

    void listUsers() const
    {
        for (const auto& u : users)
            u.print();
    }

    void statistics() const
    {
        int total = books.size();
        int borrowed = 0;

        for (const auto& b : books)
            if (b.isBorrowed())
                borrowed++;

        cout << "Total books: " << total << endl;
        cout << "Borrowed: " << borrowed << endl;
        cout << "Available: " << total - borrowed << endl;
        cout << "Users: " << users.size() << endl;
    }

    void save()
    {
        ofstream bf("books.dat");
        ofstream uf("users.dat");

        for (const auto& b : books)
            bf << b.serialize() << endl;

        for (const auto& u : users)
            uf << u.serialize() << endl;

        bf.close();
        uf.close();
    }

    void load()
    {
        ifstream bf("books.dat");
        ifstream uf("users.dat");

        string line;

        while (getline(bf, line))
        {
            if (!line.empty())
            {
                Book b = Book::deserialize(line);
                books.push_back(b);
                nextBookId = max(nextBookId, b.getId() + 1);
            }
        }

        while (getline(uf, line))
        {
            if (!line.empty())
            {
                User u = User::deserialize(line);
                users.push_back(u);
                nextUserId = max(nextUserId, u.getId() + 1);
            }
        }

        bf.close();
        uf.close();
    }

    void menu()
    {
        cout << "\n===== Library System =====\n";
        cout << "1. Add Book\n";
        cout << "2. Remove Book\n";
        cout << "3. List Books\n";
        cout << "4. Search Books\n";
        cout << "5. Add User\n";
        cout << "6. List Users\n";
        cout << "7. Borrow Book\n";
        cout << "8. Return Book\n";
        cout << "9. Statistics\n";
        cout << "0. Exit\n";
        cout << "Select: ";
    }

    void run()
    {
        int choice;

        while (true)
        {
            menu();
            cin >> choice;

            switch (choice)
            {
            case 1:
                addBook();
                break;

            case 2:
                removeBook();
                break;

            case 3:
                listBooks();
                break;

            case 4:
                searchBooks();
                break;

            case 5:
                addUser();
                break;

            case 6:
                listUsers();
                break;

            case 7:
                borrowBook();
                break;

            case 8:
                returnBook();
                break;

            case 9:
                statistics();
                break;

            case 0:
                save();
                cout << "Goodbye.\n";
                return;

            default:
                cout << "Invalid.\n";
            }

            Utils::pause();
        }
    }
};

// Main
int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Library lib;
    lib.run();

    return 0;
}
