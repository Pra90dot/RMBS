#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <climits>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#endif
using namespace std;
const int MAX = 100;      
const int MAX_NAME = 40;  
const int MAX_QTY = 1000; 
struct Item
{
    string name;
    double price;
};
struct Order
{
    string name;
    double price;
    int qty;
};
vector<Item> menu;
vector<Order> orders;
string trim(const string &s)
{
    size_t start = 0;
    size_t end = s.size();
    while (start < end && isspace((unsigned char)s[start]))
        start++;
    while (end > start && isspace((unsigned char)s[end - 1]))
        end--;
    return s.substr(start, end - start);
}
bool equalsIgnoreCase(const string &a, const string &b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); i++)
    {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
            return false;
    }
    return true;
}
string readLine()
{
    string line;
    if (!getline(cin, line))
    {
        cout << "\nInput closed. Program Exited.\n";
        exit(0);
    }
    return trim(line);
}
bool parseInt(const string &s, int &value)
{
    if (s.empty())
        return false;
    try
    {
        size_t pos = 0;
        long long x = stoll(s, &pos);
        if (pos != s.size() || x < INT_MIN || x > INT_MAX)
            return false;
        value = (int)x;
        return true;
    }
    catch (...)
    {
        return false;
    }
}
bool parseDouble(const string &s, double &value)
{
    if (s.empty())
        return false;
    char *end = NULL;
    double v = strtod(s.c_str(), &end);
    if (*end != '\0' || !isfinite(v))
        return false;
    value = v;
    return true;
}
int readInt(const string &prompt, int lo, int hi)
{
    while (true)
    {
        cout << prompt;
        string s = readLine();
        int v;
        if (parseInt(s, v) && v >= lo && v <= hi)
            return v;
        cout << "Invalid input. Enter a number from " << lo << " to " << hi << ".\n";
    }
}
bool readPrice(const string &prompt, double &price)
{
    while (true)
    {
        cout << prompt;
        string s = readLine();
        if (s == "0")
            return false;
        double v;
        if (!parseDouble(s, v) || v <= 0 || v > 1000000)
        {
            cout << "Invalid price. Enter a positive amount (or 0 to go back).\n";
            continue;
        }
        price = round(v * 100.0) / 100.0;
        return true;
    }
}
bool confirm(const string &prompt)
{
    while (true)
    {
        cout << prompt << " (y/n): ";
        string s = readLine();
        if (equalsIgnoreCase(s, "y") || equalsIgnoreCase(s, "yes"))
            return true;
        if (equalsIgnoreCase(s, "n") || equalsIgnoreCase(s, "no"))
            return false;
        cout << "Please type y or n.\n";
    }
}

string readPassword(const string &prompt)
{
    cout << prompt << flush;
    string pw;

#ifdef _WIN32
    while (true)
    {
        int c = _getch();
        if (c == '\r' || c == '\n')
            break;
        if (c == 3) 
        {
            cout << "\n";
            exit(0);
        }
        if (c == 0 || c == 224) 
        {
            _getch();
            continue;
        }
        if (c == 8) 
        {
            if (!pw.empty())
            {
                pw.pop_back();
                cout << "\b \b" << flush;
            }
        }
        else if (c >= 32 && c < 127)
        {
            pw += (char)c;
            cout << '*' << flush;
        }
    }
    cout << "\n";
#else
    termios oldt;
    if (tcgetattr(STDIN_FILENO, &oldt) != 0)
    {
        return readLine();
    }
    termios newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (true)
    {
        int c = getchar();
        if (c == EOF || c == '\n' || c == '\r')
            break;
        if (c == 127 || c == 8)
        {
            if (!pw.empty())
            {
                pw.pop_back();
                cout << "\b \b" << flush;
            }
        }
        else if (c >= 32 && c < 127)
        {
            pw += (char)c;
            cout << '*' << flush;
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << "\n";
#endif

    return pw;
}
enum LoginResult
{
    LOGIN_OK,
    LOGIN_FAILED,
    LOGIN_CANCELLED
};

LoginResult login(const string &user, const string &pass)
{
    const int MAX_ATTEMPTS = 3;
    cout << "\n(Enter 0 as username to go back)\n";
    for (int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++)
    {
        cout << "Username: ";
        string u = readLine();
        if (u == "0")
            return LOGIN_CANCELLED;
        string p = readPassword("Password: ");
        if (u == user && p == pass)
            return LOGIN_OK;
        cout << "Invalid login.";
        if (attempt < MAX_ATTEMPTS)
            cout << " Attempts left: " << (MAX_ATTEMPTS - attempt);
        cout << "\n";
    }
    return LOGIN_FAILED;
}
void loadMenu()
{
    menu.clear();
    ifstream fp("menu.txt");
    if (!fp)
        return;
    string line;
    while ((int)menu.size() < MAX && getline(fp, line))
    {
        line = trim(line);
        if (line.empty())
            continue;
        size_t pos = line.rfind(',');
        if (pos == string::npos)
            continue;
        Item item;
        item.name = trim(line.substr(0, pos));
        if (item.name.empty() || !parseDouble(trim(line.substr(pos + 1)), item.price))
            continue;
        if (item.price < 0)
            continue;
        menu.push_back(item);
    }
}

void saveMenu()
{
    ofstream fp("menu.txt");
    if (!fp)
    {
        cout << "Warning: could not save menu.txt\n";
        return;
    }
    fp << fixed << setprecision(2);
    for (size_t i = 0; i < menu.size(); i++)
        fp << menu[i].name << "," << menu[i].price << "\n";
}

void loadOrders()
{
    orders.clear();
    ifstream fp("orders.txt");
    if (!fp)
        return;
    string line;
    while ((int)orders.size() < MAX && getline(fp, line))
    {
        line = trim(line);
        if (line.empty())
            continue;
        size_t pos2 = line.rfind(',');
        if (pos2 == string::npos || pos2 == 0)
            continue;
        size_t pos1 = line.rfind(',', pos2 - 1);
        if (pos1 == string::npos)
            continue;
        Order o;
        o.name = trim(line.substr(0, pos1));
        if (o.name.empty() ||
            !parseDouble(trim(line.substr(pos1 + 1, pos2 - pos1 - 1)), o.price) ||
            !parseInt(trim(line.substr(pos2 + 1)), o.qty))
            continue;
        if (o.price < 0 || o.qty <= 0)
            continue;
        orders.push_back(o);
    }
}

void saveOrders()
{
    ofstream fp("orders.txt");
    if (!fp)
    {
        cout << "Warning: could not save orders.txt\n";
        return;
    }
    fp << fixed << setprecision(2);
    for (size_t i = 0; i < orders.size(); i++)
        fp << orders[i].name << "," << orders[i].price << "," << orders[i].qty << "\n";
}
int findMenu(const string &name)
{
    for (size_t i = 0; i < menu.size(); i++)
    {
        if (equalsIgnoreCase(menu[i].name, name))
            return (int)i;
    }
    return -1;
}
int findOrder(const string &name, double price)
{
    for (size_t i = 0; i < orders.size(); i++)
    {
        if (equalsIgnoreCase(orders[i].name, name) &&
            fabs(orders[i].price - price) < 0.005)
            return (int)i;
    }
    return -1;
}
void showMenu()
{
    cout << "\n===== MENU =====\n";
    if (menu.empty())
    {
        cout << "No items available.\n";
        return;
    }
    for (size_t i = 0; i < menu.size(); i++)
    {
        cout << (i + 1) << ". " << left << setw(20) << menu[i].name
             << " Rs " << menu[i].price << "\n";
    }
}

void showOrders()
{
    cout << "\n===== ORDERS =====\n";
    if (orders.empty())
    {
        cout << "No orders.\n";
        return;
    }
    for (size_t i = 0; i < orders.size(); i++)
    {
        cout << (i + 1) << ". " << left << setw(20) << orders[i].name
             << " x" << orders[i].qty
             << "  (Rs " << orders[i].price << " each)\n";
    }
}
void addItem()
{
    if ((int)menu.size() >= MAX)
    {
        cout << "Menu full.\n";
        return;
    }

    Item item;
    while (true)
    {
        cout << "Item Name (0 to go back): ";
        item.name = readLine();
        if (item.name == "0")
            return;
        if (item.name.empty())
        {
            cout << "Invalid name.\n";
            continue;
        }
        if (item.name.find(',') != string::npos)
        {
            cout << "Name cannot contain a comma.\n";
            continue;
        }
        if ((int)item.name.size() > MAX_NAME)
        {
            cout << "Name too long (max " << MAX_NAME << " characters).\n";
            continue;
        }
        if (findMenu(item.name) != -1)
        {
            cout << "Item already exists.\n";
            continue;
        }
        break;
    }

    if (!readPrice("Price (0 to go back): ", item.price))
        return;

    menu.push_back(item);
    saveMenu();
    cout << "Item added.\n";
}

void updatePrice()
{
    if (menu.empty())
    {
        cout << "No items to update.\n";
        return;
    }
    showMenu();
    int n = readInt("Item Number (0 to go back): ", 0, (int)menu.size());
    if (n == 0)
        return;

    double price;
    if (!readPrice("New Price for " + menu[n - 1].name + " (0 to go back): ", price))
        return;

    menu[n - 1].price = price;
    saveMenu();
    cout << "Updated.\n";
}

void deleteItem()
{
    if (menu.empty())
    {
        cout << "No items to delete.\n";
        return;
    }
    showMenu();
    int n = readInt("Delete Number (0 to go back): ", 0, (int)menu.size());
    if (n == 0)
        return;

    if (!confirm("Delete '" + menu[n - 1].name + "'?"))
    {
        cout << "Cancelled.\n";
        return;
    }
    menu.erase(menu.begin() + (n - 1));
    saveMenu();
    cout << "Deleted.\n";
}
void adminPanel()
{
    while (true)
    {
        cout << "\n===== ADMIN =====\n";
        cout << "1. View Menu\n";
        cout << "2. Add Item\n";
        cout << "3. Update Price\n";
        cout << "4. Delete Item\n";
        cout << "0. Back\n";
        int choice = readInt("Choice: ", 0, 4);
        if (choice == 0)
            return;
        else if (choice == 1)
            showMenu();
        else if (choice == 2)
            addItem();
        else if (choice == 3)
            updatePrice();
        else if (choice == 4)
            deleteItem();
    }
}
double orderTotal()
{
    double total = 0;
    for (size_t i = 0; i < orders.size(); i++)
        total += orders[i].price * orders[i].qty;
    return total;
}
void generateBill()
{
    cout << "\n========== BILL ==========\n";
    for (size_t i = 0; i < orders.size(); i++)
    {
        double sub = orders[i].price * orders[i].qty;
        cout << left << setw(20) << orders[i].name
             << " x" << left << setw(4) << orders[i].qty
             << "= " << sub << "\n";
    }
    cout << "--------------------------\n";
    cout << "TOTAL = Rs " << orderTotal() << "\n";
}
void orderItem()
{
    if (menu.empty())
    {
        cout << "Menu is empty. Ask admin to add items.\n";
        return;
    }
    while (true)
    {
        showMenu();
        int itemNo = readInt("Item Number (0 to go back): ", 0, (int)menu.size());
        if (itemNo == 0)
            return;

        const Item &item = menu[itemNo - 1];
        int qty = readInt("Quantity of " + item.name + " (0 to pick another item): ",
                          0, MAX_QTY);
        if (qty == 0)
            continue;

        int id = findOrder(item.name, item.price);
        if (id != -1 && orders[id].qty + qty > MAX_QTY)
        {
            cout << "Total quantity for this item cannot exceed " << MAX_QTY << ".\n";
            continue;
        }
        if (id == -1 && (int)orders.size() >= MAX)
        {
            cout << "Order full.\n";
            return;
        }
        if (!confirm("Add " + to_string(qty) + " x " + item.name + "?"))
        {
            cout << "Not added.\n";
            continue;
        }

        if (id != -1)
        {
            orders[id].qty += qty;
        }
        else
        {
            Order o;
            o.name = item.name;
            o.price = item.price;
            o.qty = qty;
            orders.push_back(o);
        }
        saveOrders();
        cout << "Order added.\n";
        return;
    }
}
void cancelOrder()
{
    showOrders();
    if (orders.empty())
        return;

    int n = readInt("Order Number (0 to go back): ", 0, (int)orders.size());
    if (n == 0)
        return;

    Order &o = orders[n - 1];
    int qty = readInt("Cancel Quantity (0 to go back, " + to_string(o.qty) + " on order): ",
                      0, MAX_QTY);
    if (qty == 0)
        return;

    bool removeAll = qty >= o.qty;
    string msg = removeAll
                     ? "Remove the whole order line (" + to_string(o.qty) + " x " + o.name + ")?"
                     : "Cancel " + to_string(qty) + " x " + o.name + "?";
    if (!confirm(msg))
    {
        cout << "Cancelled. Nothing changed.\n";
        return;
    }

    if (removeAll)
        orders.erase(orders.begin() + (n - 1));
    else
        o.qty -= qty;

    saveOrders();
    cout << "Updated.\n";
}
void billOrders()
{
    if (orders.empty())
    {
        cout << "No orders to bill.\n";
        return;
    }
    generateBill();
    if (!confirm("Finish billing and clear all orders?"))
    {
        cout << "Billing not finished. Orders kept.\n";
        return;
    }
    orders.clear();
    saveOrders();
    cout << "\nOrders cleared after billing.\n";
}
void staffPanel()
{
    while (true)
    {
        cout << "\n===== STAFF =====\n";
        cout << "1. Order Item\n";
        cout << "2. View Orders\n";
        cout << "3. Cancel Order\n";
        cout << "4. Generate Bill\n";
        cout << "0. Back\n";
        int choice = readInt("Choice: ", 0, 4);

        if (choice == 0)
            return;
        else if (choice == 1)
            orderItem();
        else if (choice == 2)
            showOrders();
        else if (choice == 3)
            cancelOrder();
        else if (choice == 4)
            billOrders();
    }
}
int main()
{
    const string adminUser = "admin";
    const string adminPass = "1234";
    const string staffUser = "staff";
    const string staffPass = "1111";
    cout << fixed << setprecision(2);
    loadMenu();
    loadOrders();
    while (true)
    {
        cout << "\n===== RESTAURANT SYSTEM =====\n";
        cout << "1. Admin Login\n";
        cout << "2. Staff Login\n";
        cout << "0. Exit\n";
        int choice = readInt("Choice: ", 0, 2);
        if (choice == 0)
        {
            if (confirm("Exit program?"))
                break;
        }
        else if (choice == 1)
        {
            LoginResult r = login(adminUser, adminPass);
            if (r == LOGIN_OK)
                adminPanel();
            else if (r == LOGIN_FAILED)
                cout << "Too many failed attempts.\n";
        }
        else if (choice == 2)
        {
            LoginResult r = login(staffUser, staffPass);
            if (r == LOGIN_OK)
                staffPanel();
            else if (r == LOGIN_FAILED)
                cout << "Too many failed attempts.\n";
        }
    }
    cout << "Program Exited.\n";
    return 0;
}
