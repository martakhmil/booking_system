#include<iostream>
#include<vector>
#include<string>
#include<memory>
#include<limits>
#include<iomanip>
#include<sstream>
#include<algorithm>
#include<map>
#include<cassert>
#include "crow_all.h"
#include"sqlite3.h"
#ifdef _WIN32
#include<windows.h>
#undef max
#endif
using namespace std;


template<typename K,typename V>
class Dictionary{
    map<K,V> data;
public:
    void add(K key,V value){
        data[key]=value;}
    void show() const{
        for(auto x:data){
            cout<<x.first<<" -> "<<x.second<<"\n";}}
};

class Bookable{
public:
    virtual ~Bookable(){}
    virtual int getId() const=0;
    virtual string info() const=0;
    virtual bool isFree() const=0;
    virtual bool book(const string& client,const string& date)=0;
    virtual bool cancel()=0;
};

class Priced{
public:
    virtual ~Priced(){}
    virtual double price() const=0;
};

class ResourceBase{
protected:
    int id;
    bool free;
    string client;
    string date;
    ResourceBase(int id):id(id),free(true){}
};

enum class RoomType{
    STANDARD=1,
    LUX=2
};

enum class Role{
    USER=1,
    ADMIN=2
};

static string roomTypeName(RoomType t){
    return t==RoomType::STANDARD?"СТАНДАРТ":"ЛЮКС";
}

class BookingSystem{
public:
    class Validator{
    public:
        static bool isValidPrice(double p){
            return p>0;}
    };
};

class HotelRoom:public Bookable,public Priced,protected ResourceBase{
    RoomType type;
    double p;
public:
    HotelRoom(int id,RoomType type,double p):ResourceBase(id),type(type),p(p){}
    int getId() const override{
        return id;}
    double price() const override{
        return p;}
    bool isFree() const override{
        return free;}
    bool book(const string& c,const string& d) override{
        if(!free){
            return false;
        }
        free=false;
        client=c;
        date=d;
        return true;}
    bool cancel() override{
        if(free){
            return false;
        }
        free=true;
        client.clear();
        date.clear();
        return true;}
    string info() const override{
        ostringstream out;
        out<<fixed<<setprecision(2);
        out<<"готельний номер | id="<<id<<" | тип="<<roomTypeName(type)<<" | ціна="<<p;
        if(!free){
            out<<" | клієнт=\""<<client<<"\" | дата=\""<<date<<"\"";}
        return out.str();}
};

class RestaurantTable:public Bookable,public Priced,protected ResourceBase{
    int seats;
    double p;
public:
    RestaurantTable(int id,int seats,double p):ResourceBase(id),seats(seats),p(p){}
    int getId() const override{
        return id;}
    double price() const override{
        return p;}
    bool isFree() const override{
        return free;}
    bool book(const string& c,const string& d) override{
        if(!free){
            return false;}
        free=false;
        client=c;
        date=d;
        return true;}
    bool cancel() override{
        if(free){
            return false;}
        free=true;
        client.clear();
        date.clear();
        return true;}
    string info() const override{
        ostringstream out;
        out<<fixed<<setprecision(2);
        out<<"ресторанний столик | id="<<id<<" | місця="<<seats<<" | ціна="<<p;
        if(!free){
            out<<" | клієнт=\""<<client<<"\" | дата=\""<<date<<"\"";}
        return out.str();}
};

class ResourceFactory{
public:
    static unique_ptr<Bookable> createRoom(int id,RoomType type,double price){
        return make_unique<HotelRoom>(id,type,price);}
    static unique_ptr<Bookable> createTable(int id,int seats,double price){
        return make_unique<RestaurantTable>(id,seats,price);}
};

class SortStrategy{
public:
    virtual ~SortStrategy(){}
    virtual void sortItems(vector<unique_ptr<Bookable>>& items)=0;
};

class SortById:public SortStrategy{
public:

    void sortItems(
        vector<unique_ptr<Bookable>>& items
    ) override{

        sort(
            items.begin(),
            items.end(),

            [](const unique_ptr<Bookable>& a,
               const unique_ptr<Bookable>& b){

                bool roomA=
                dynamic_cast<HotelRoom*>(a.get());

                bool roomB=
                dynamic_cast<HotelRoom*>(b.get());

                if(roomA!=roomB){
                    return roomA>roomB;
                }

                return a->getId()<b->getId();
            }
        );
    }
};

class Booking{
    int bId;
    int rId;
    string cl;
    string dt;
public:
    Booking(){}
    Booking(int bId,int rId,const string& cl,const string& dt){
        this->bId=bId;
        this->rId=rId;
        this->cl=cl;
        this->dt=dt;
    }
    int getBookingId() const{
    return bId;
}
    int getResourceId() const{
        return rId;}
    string getClient() const{
        return cl;}
    string getDate() const{
        return dt;}
    string str() const{
        return "бронювання #"+to_string(bId)+" | ресурс id="+to_string(rId)+" | клієнт=\""+cl+"\" | дата=\""+dt+"\"";
    }
};

ostream& operator<<(ostream& out,const Booking& b){
    out<<b.str();
    return out;}

class IBookingRepository{
public:
    virtual ~IBookingRepository(){}
    virtual void add(const Booking& b)=0;
    virtual vector<Booking> getAll() const=0;
};

class SQLiteBookingRepository:public IBookingRepository{
    sqlite3* db;

    SQLiteBookingRepository(){

        sqlite3_open("/app/hotel.db",&db);

        string sql=
        "CREATE TABLE IF NOT EXISTS bookings("
        "id INTEGER,"
        "resourceId INTEGER,"
        "client TEXT,"
        "date TEXT);";

        sqlite3_exec(db,sql.c_str(),0,0,0);

        string sql2=
        "CREATE TABLE IF NOT EXISTS resources("
        "id INTEGER,"
        "type TEXT,"
        "price REAL,"
        "free INTEGER,"
        "extra TEXT);";

        sqlite3_exec(db,sql2.c_str(),0,0,0);
    }

public:

    static SQLiteBookingRepository& getInstance(){

        static SQLiteBookingRepository instance;

        return instance;
    }

    SQLiteBookingRepository(
        const SQLiteBookingRepository&
    )=delete;

    void operator=(
        const SQLiteBookingRepository&
    )=delete;

    ~SQLiteBookingRepository(){

        sqlite3_close(db);
    }

    void add(const Booking& b) override{

        string sql=
        "INSERT INTO bookings VALUES("
        +to_string(b.getBookingId())+","
        +to_string(b.getResourceId())+",'"
        +b.getClient()+"','"
        +b.getDate()+"');";

        sqlite3_exec(
            db,
            sql.c_str(),
            0,
            0,
            0
        );
    }

    void addResource(
    int id,
    const string& type,
    double price,
    int free,
    const string& extra
){

        string sql=
        "INSERT INTO resources VALUES("
        +to_string(id)+",'"
        +type+"',"
        +to_string(price)+","
        +to_string(free)+",'"
        +extra+"');";

        sqlite3_exec(
            db,
            sql.c_str(),
            0,
            0,
            0
        );
    }

    vector<Booking> getAll() const override{

        vector<Booking> res;

        string sql=
        "SELECT * FROM bookings;";

        sqlite3_stmt* stmt;

        sqlite3_prepare_v2(
            db,
            sql.c_str(),
            -1,
            &stmt,
            0
        );

        while(sqlite3_step(stmt)==SQLITE_ROW){

            int id=
            sqlite3_column_int(stmt,0);

            int rid=
            sqlite3_column_int(stmt,1);

            string cl=
            (char*)sqlite3_column_text(stmt,2);

            string dt=
            (char*)sqlite3_column_text(stmt,3);

            res.push_back(
                Booking(id,rid,cl,dt)
            );
        }

        sqlite3_finalize(stmt);

        return res;
    }

    void removeById(int id){

    int resourceId=-1;

    string sql1=
    "SELECT resourceId FROM bookings WHERE id="
    +to_string(id)+";";

    sqlite3_stmt* stmt;

    sqlite3_prepare_v2(
        db,
        sql1.c_str(),
        -1,
        &stmt,
        0
    );

    if(sqlite3_step(stmt)==SQLITE_ROW){

        resourceId=
        sqlite3_column_int(stmt,0);
    }

    sqlite3_finalize(stmt);

    string sql2=
    "DELETE FROM bookings WHERE id="
    +to_string(id)+";";

    sqlite3_exec(
        db,
        sql2.c_str(),
        0,
        0,
        0
    );

    if(resourceId!=-1){

        updateFree(resourceId,1);
    }
}

    vector<tuple<int,string,double,int,string>>
getResources() const{

    vector<
        tuple<int,string,double,int,string>
    > res;

        string sql=
        "SELECT * FROM resources "
        "ORDER BY type DESC,id ASC;";

        sqlite3_stmt* stmt;

        sqlite3_prepare_v2(
            db,
            sql.c_str(),
            -1,
            &stmt,
            0
        );

        while(sqlite3_step(stmt)==SQLITE_ROW){

            int id=
            sqlite3_column_int(stmt,0);

            string type=
            (char*)sqlite3_column_text(stmt,1);

            double price=
            sqlite3_column_double(stmt,2);

            int free=
            sqlite3_column_int(stmt,3);

            string extra=
            (char*)sqlite3_column_text(stmt,4);

            res.push_back({
    id,
    type,
    price,
    free,
    extra
});
        }

        sqlite3_finalize(stmt);

        return res;
    }

    void updateFree(
        int id,
        int free
    ){

        string sql=
        "UPDATE resources SET free="
        +to_string(free)+
        " WHERE id="
        +to_string(id)+";";

        sqlite3_exec(
            db,
            sql.c_str(),
            0,
            0,
            0
        );
    }

    void deleteResource(int id){

    string sql=
    "DELETE FROM resources WHERE id="
    +to_string(id)+";";

    sqlite3_exec(
        db,
        sql.c_str(),
        0,
        0,
        0
    );
}

void deleteBookingsByResource(int id){

    string sql=
    "DELETE FROM bookings WHERE resourceId="
    +to_string(id)+";";

    sqlite3_exec(
        db,
        sql.c_str(),
        0,
        0,
        0
    );
}


void updateResourcePrice(
    int id,
    double price
){

    string sql=
    "UPDATE resources SET price="
    +to_string(price)+
    " WHERE id="
    +to_string(id)+";";

    sqlite3_exec(
        db,
        sql.c_str(),
        0,
        0,
        0
    );
} 


    void loadResources(vector<unique_ptr<Bookable>>& items){

    auto all=getResources();

    for(auto x:all){

        int id=get<0>(x);

        string type=get<1>(x);

        double price=get<2>(x);

        int free=get<3>(x);
        string extra=get<4>(x);

        if(type=="room"){

            items.push_back(
                ResourceFactory::createRoom(
                    id,
                    extra=="lux"
?RoomType::LUX
:RoomType::STANDARD,
                    price
                )
            );
        }

        else{

            items.push_back(
                ResourceFactory::createTable(
                    id,
                    stoi(extra),
                    price
                )
            );
        }

        if(free==0){

    auto bookings=
    getAll();

    for(auto b:bookings){

        if(b.getResourceId()==id){

            items.back()->book(
                b.getClient(),
                b.getDate()
            );

            break;
        }
    }
}
    }
}
};

class BookingService{
    IBookingRepository& repo;
public:
    BookingService(IBookingRepository& repo):repo(repo){}
    void createBooking(const Booking& b){
        repo.add(b);}
    void showBookings(){
        vector<Booking> all=repo.getAll();
        if(all.empty()){
            cout<<"бронювань немає\n";}
        for(auto x:all){
            cout<<x<<"\n";}
    }
};

class Container{
public:
    BookingService getBookingService(){
        return BookingService(
            SQLiteBookingRepository::getInstance());
    }
};

static int findResIdx(const vector<unique_ptr<Bookable>>& items,int id){
    for(int i=0;i<(int)items.size();i++){
        if(items[i]->getId()==id){
            return i;}
    }
    return -1;
}

static void showResources(vector<unique_ptr<Bookable>>& items,SortStrategy& strategy){
    strategy.sortItems(items);
    if(items.empty()){
        cout<<"ресурсів немає\n";}
    for(auto& x:items){
        cout<<x->info()<<"\n";}
}

static void doBook(vector<unique_ptr<Bookable>>& items,BookingService& service,int& nextB){
    int id;
    cout<<"id ресурсу: ";
    cin>>id;
    cin.ignore(
        std::numeric_limits<std::streamsize>::max(),
        '\n');

    int idx=findResIdx(items,id);

    if(idx==-1 || !items[idx]->isFree()){
        cout<<"неможливо\n";
        return;}

    string c,d;
    cout<<"клієнт: ";
    getline(cin,c);
    cout<<"дата: ";
    getline(cin,d);

    if(items[idx]->book(c,d)){
        service.createBooking(
            Booking(nextB++,id,c,d));
            SQLiteBookingRepository::getInstance()
            .updateFree(id,0);
        cout<<"успішно\n";
    }
}

static void doAddRoom(vector<unique_ptr<Bookable>>& items,int& nxt){
    cout<<"1-стандарт, 2-люкс: ";
    int t;
    cin>>t;
    double pr;
    cout<<"ціна: ";
    cin>>pr;

    if(BookingSystem::Validator::isValidPrice(pr)){

        items.push_back(
            ResourceFactory::createRoom(
                nxt++,
                t==1?RoomType::STANDARD:RoomType::LUX,
                pr));

        SQLiteBookingRepository::getInstance().addResource(
    nxt-1,
    "room",
    pr,
    1,
    t==1?"standard":"lux"
);
        cout<<"додано\n";
    }
}

static void doAddTable(vector<unique_ptr<Bookable>>& items,int& nxt){
    int s;
    cout<<"місця: ";
    cin>>s;
    double pr;
    cout<<"ціна: ";
    cin>>pr;

    if(BookingSystem::Validator::isValidPrice(pr)){
        items.push_back(
            ResourceFactory::createTable(
                nxt++,
                s,
                pr ));

        SQLiteBookingRepository::getInstance().addResource(
    nxt-1,
    "table",
    pr,
    1,
    to_string(s)
);
        cout<<"додано\n";
    }
}

static Role readRole(){
    while(true){
        cout<<"Оберіть роль:\n1 користувач\n2 адмін\n";
        int r;
        if(cin>>r && (r==1 || r==2)){
            cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n' );

            return r==1
            ?Role::USER
            :Role::ADMIN;
        }

        cout<<"помилка\n";
        cin.clear();
        cin.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n' );
    }
}

void runTests(){
    assert(
        BookingSystem::Validator::isValidPrice(100)
        ==true);

    assert(
        BookingSystem::Validator::isValidPrice(-5)
        ==false);

    HotelRoom room(
        1,
        RoomType::STANDARD,
        1000);

    assert(room.isFree()==true);

    room.book("Іван","11.05");

    assert(room.isFree()==false);

    room.cancel();

    assert(room.isFree()==true);

    cout<<"усі тести пройдені\n\n";
}

void addCors(crow::response& res){
    res.add_header("Access-Control-Allow-Origin","*");
    res.add_header("Access-Control-Allow-Methods","GET, POST, PUT, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers","Content-Type");
}

int main(){

#ifdef _WIN32
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
#endif

    runTests();

    Container container;

    BookingService service=
    container.getBookingService();

    vector<unique_ptr<Bookable>> items;

    SQLiteBookingRepository::getInstance()
    .loadResources(items);

    SortById sorter;

    int nxtR=1;

for(auto& x:items){

    nxtR=max(
        nxtR,
        x->getId()+1
    );
}

auto allBookings=
SQLiteBookingRepository::
getInstance()
.getAll();

int nxtB=1;

for(auto x:allBookings){

    nxtB=max(
        nxtB,
        x.getBookingId()+1
    );
}


    crow::SimpleApp app;

    CROW_ROUTE(app,"/<path>")
.methods("OPTIONS"_method)
([](string path){

    crow::response res;

    addCors(res);

    res.code=200;

    return res;
});

    CROW_ROUTE(app,"/")
([](){

    crow::response res(
        "booking system api"
    );

    addCors(res);

    return res;
});

    CROW_ROUTE(app,"/resources")
    ([&items,&sorter](){

        sorter.sortItems(items);

        crow::json::wvalue result;

        int i=0;

        for(auto& x:items){

            result[i]["id"]=
            x->getId();

            result[i]["info"]=
            x->info();

            result[i]["free"]=
            x->isFree();

            i++;
        }

        crow::response res(result);

addCors(res);

return res;
    });

    CROW_ROUTE(app,"/bookings")
    ([&](){

        vector<Booking> all=
        SQLiteBookingRepository::
        getInstance()
        .getAll();

        crow::json::wvalue result;

        int i=0;

        for(auto x:all){

            result[i]["bookingId"]=
            x.getBookingId();

            result[i]["resourceId"]=
            x.getResourceId();

            result[i]["client"]=
            x.getClient();

            result[i]["date"]=
            x.getDate();

            i++;
        }

       crow::response res(result);

addCors(res);

return res;
    });
CROW_ROUTE(app,"/book")
.methods("OPTIONS"_method)
([](){
    crow::response res;
    addCors(res);
    return res;
});
    CROW_ROUTE(app,"/book")
    .methods("POST"_method)

    ([&items,&service,&nxtB]
    (const crow::request& req){

        auto body=
        crow::json::load(req.body);

        if(!body){

             crow::response res(
                400,
                "invalid json"
            );
            addCors(res);

return res;
        }

        int id=
        body["id"].i();

        string client=
        body["client"].s();

        string date=
        body["date"].s();

        int idx=
        findResIdx(items,id);

        if(idx==-1){

            crow::response res(
                404,
                "resource not found"
            );
           addCors(res); 

return res;
        }

        if(!items[idx]->isFree()){

            crow::response res(
                400,
                "resource busy"
            );
         addCors(res);  

return res;
        }

        if(items[idx]->book(client,date)){

            service.createBooking(
                Booking( nxtB++,  id, client, date )
            );

            SQLiteBookingRepository::
            getInstance()
            .updateFree(id,0);

            crow::response res(
                200,
                "success"
            );
           addCors(res);

return res;
        }

        crow::response res(
            400,
            "booking failed"
        );
        addCors(res);

return res;
    });
CROW_ROUTE(app,"/add-room")
.methods("OPTIONS"_method)
([](){
    crow::response res;
    addCors(res);
    return res;
});
    CROW_ROUTE(app,"/add-room")
    .methods("POST"_method)

    ([&items,&nxtR]
    (const crow::request& req){

        auto body=
        crow::json::load(req.body);

        if(!body){

             crow::response res(
                400,
                "invalid json"
            );
        addCors(res);   

return res;
        }

        int type=
        body["type"].i();

        double price=
        body["price"].d();

        if(!BookingSystem::
        Validator::
        isValidPrice(price)){

            crow::response res(
                400,
                "invalid price"
            );
        addCors(res);   

return res;
        }

        items.push_back(
            ResourceFactory::
            createRoom(  nxtR, type==1  ?RoomType::STANDARD :RoomType::LUX,price )
        );

        SQLiteBookingRepository::
        getInstance()
        .addResource( nxtR, "room", price,1,type==1   ?"standard"  :"lux" );

        nxtR++;

       crow::response res(
            200,
            "room added"
        );
       addCors(res);

return res;
    });
CROW_ROUTE(app,"/add-table")
.methods("OPTIONS"_method)
([](){
    crow::response res;
    addCors(res);
    return res;
});
    CROW_ROUTE(app,"/add-table")
    .methods("POST"_method)

    ([&items,&nxtR]
    (const crow::request& req){

        auto body=
        crow::json::load(req.body);

        if(!body){

            crow::response res(
                400,
                "invalid json"
            );
          addCors(res); 

return res;
        }

        int seats=
        body["seats"].i();

        double price=
        body["price"].d();

        if(!BookingSystem::
        Validator::
        isValidPrice(price)){

            crow::response res(
                400,
                "invalid price"
            );
           addCors(res);

return res;
        }

        items.push_back(
            ResourceFactory::
            createTable( nxtR,seats, price )
        );

        SQLiteBookingRepository::
        getInstance()
        .addResource( nxtR,  "table",  price, 1,to_string(seats) );

        nxtR++;

        crow::response res(
            200,
            "table added"
        );
       addCors(res);

return res;
    });
CROW_ROUTE(app,"/booking/<int>")
.methods("OPTIONS"_method)
([](int id){
    crow::response res;
    addCors(res);
    return res;
});
    CROW_ROUTE(app,"/booking/<int>")
    .methods("DELETE"_method)

    ([](int id){

        SQLiteBookingRepository::
        getInstance()
        .removeById(id);

         crow::response res(
            200,
            "deleted"
        );
       addCors(res);

return res;
    });

    CROW_ROUTE(app,"/resources/<int>")
([&items](int id){

    int idx=
    findResIdx(items,id);

    if(idx==-1){

        crow::response res(
            404,
            "resource not found"
        );
       addCors(res);

return res;
    }

    crow::json::wvalue x;

    x["id"]=
    items[idx]->getId();

    x["info"]=
    items[idx]->info();

    x["free"]=
    items[idx]->isFree();

    crow::response res(x);

addCors(res);

return res;
});

CROW_ROUTE(app,"/bookings/<int>")
([](int id){

    auto all=
    SQLiteBookingRepository::
    getInstance()
    .getAll();

    for(auto x:all){

        if(x.getBookingId()==id){

            crow::json::wvalue r;

            r["bookingId"]=
            x.getBookingId();

            r["resourceId"]=
            x.getResourceId();

            r["client"]=
            x.getClient();

            r["date"]=
            x.getDate();

            crow::response res(r);

addCors(res);

return res;
        }
    }

     crow::response res(
        404,
        "booking not found"
    );
   addCors(res);

return res;
});

CROW_ROUTE(app,"/booking/<int>")
.methods("PUT"_method)

([&](const crow::request& req,int id){

    auto body= crow::json::load(req.body);

    if(!body){
         crow::response res(
            400,
            "invalid json"
        );
    addCors(res);

return res;
    }

    string client=body["client"].s();

    string date=body["date"].s();

    auto all=
    SQLiteBookingRepository::
    getInstance()
    .getAll();

    int resourceId=-1;

    for(auto x:all){

        if(x.getBookingId()==id){
            resourceId=
            x.getResourceId();
        }
    }

    if(resourceId==-1){
        crow::response res(
            404,
            "booking not found"
        );
     addCors(res);

return res;
    }

    SQLiteBookingRepository::
    getInstance()
    .removeById(id);

    SQLiteBookingRepository::
    getInstance()
    .add(

         Booking(id, resourceId, client,date )
    );

    SQLiteBookingRepository::
    getInstance()
    .updateFree(resourceId,0);

 crow::response res(
        200,
        "updated"
    );
   addCors(res);

return res;
});
CROW_ROUTE(app,"/resources/<int>")
.methods("OPTIONS"_method)
([](int id){
    crow::response res;
    addCors(res);
    return res;
});
CROW_ROUTE(app,"/resources/<int>")
.methods("DELETE"_method)

([&items](int id){

    int idx=findResIdx(items,id);

    if(idx==-1){
        crow::response res(
            404,
            "resource not found"
        );
    addCors(res);

return res;
    }

    items.erase(items.begin()+idx);

SQLiteBookingRepository::
getInstance()
.deleteBookingsByResource(id);

SQLiteBookingRepository::
getInstance()
.deleteResource(id);

crow::response res(
    200,
    "resource deleted"
);
addCors(res);

return res;
});

CROW_ROUTE(app,"/resources/<int>")
.methods("PUT"_method)

([&items]
(const crow::request& req,int id){

    auto body=crow::json::load(req.body);

    if(!body){
         crow::response res(
            400,
            "invalid json"
        );
 addCors(res);

return res;
    }

    int idx=findResIdx(items,id);

    if(idx==-1){
       crow::response res(
            404,
            "resource not found");
  addCors(res);

return res;
    }

    double price=body["price"].d();

    if(!BookingSystem::Validator::isValidPrice(price)){
        crow::response res(
            400,
            "invalid price");
            addCors(res);
    

return res;
    }

    bool room=dynamic_cast<HotelRoom*>(items[idx].get());

    int oldId=items[idx]->getId();

    bool free=items[idx]->isFree();

    items.erase(items.begin()+idx);

    if(room){

        int type=body["type"].i();

        items.push_back(
            ResourceFactory::
            createRoom( oldId, type==1 ?RoomType::STANDARD:RoomType::LUX, price )
        );

        SQLiteBookingRepository::
        getInstance()
        .deleteResource(oldId);

        SQLiteBookingRepository::
        getInstance()
        .addResource( oldId,  "room", price, free?1:0, type==1 ?"standard":"lux" );
    }

    else{
        int seats=
        body["seats"].i();

        items.push_back(ResourceFactory::createTable(oldId, seats, price ));

        SQLiteBookingRepository::
        getInstance()
        .deleteResource(oldId);

        SQLiteBookingRepository::
        getInstance()
        .addResource( oldId,"table", price,  free?1:0,to_string(seats) );
    }
    crow::response res( 200, "resource updated" );
   addCors(res);

return res;
});

CROW_ROUTE(app,"/swagger")
([](){

    crow::response res;
    addCors(res);

    res.code=200;

    res.set_header(
        "Content-Type",
        "text/html"
    );

    res.body=R"(

<!DOCTYPE html>
<html>

<head>

<link rel="stylesheet"
href="https://unpkg.com/swagger-ui-dist/swagger-ui.css"/>

</head>

<body>

<div id="swagger-ui"></div>

<script src="https://unpkg.com/swagger-ui-dist/swagger-ui-bundle.js"></script>

<script>

window.onload=function(){

SwaggerUIBundle({

url:'/swagger.json',

dom_id:'#swagger-ui'

});

};

</script>

</body>
</html>

)";

    return res;
});



CROW_ROUTE(app,"/swagger.json")
([](){
crow::response res;

    addCors(res);

    res.set_header(
        "Content-Type",
        "application/json"
    );
res.body=R"(

{
  "openapi":"3.0.0",

  "info":{
    "title":"Booking System API",
    "version":"1.0",
    "description":"REST API for hotel and restaurant booking system"
  },

  "paths":{

    "/resources":{

      "get":{
        "summary":"Get all resources",

        "responses":{
          "200":{
            "description":"List of resources"
          }
        }
      }
    },

    "/resources/{id}":{

      "get":{

        "summary":"Get resource by id",

        "parameters":[

          {
            "name":"id",
            "in":"path",
            "required":true,

            "schema":{
              "type":"integer"
            }
          }
        ],

        "responses":{
          "200":{
            "description":"Resource found"
          },

          "404":{
            "description":"Resource not found"
          }
        }
      },
      "put":{

        "summary":"Update resource",

        "parameters":[

          {
            "name":"id",
            "in":"path",
            "required":true,

            "schema":{
              "type":"integer"
            }
          }
        ],

        "requestBody":{

          "required":true,

          "content":{

            "application/json":{

              "schema":{

                "type":"object",

                "properties":{

                  "price":{
                    "type":"number"
                  },

                  "type":{
                    "type":"integer"
                  },

                  "seats":{
                    "type":"integer"
                  }
                }
              },

              "example":{

                "price":3000,
                "type":1
              }
            }
          }
        },

        "responses":{

          "200":{
            "description":"Resource updated"
          }
        }
      },

      "delete":{

        "summary":"Delete resource",

        "parameters":[

          {
            "name":"id",
            "in":"path",
            "required":true,

            "schema":{
              "type":"integer"
            }
          }
        ],

        "responses":{

          "200":{
            "description":"Resource deleted"
          }
        }
      }
    },

    "/bookings":{

      "get":{

        "summary":"Get all bookings",

        "responses":{
          "200":{
            "description":"List of bookings"
          }
        }
      }
    },

    "/bookings/{id}":{

      "get":{

        "summary":"Get booking by id",

        "parameters":[

          {
            "name":"id",
            "in":"path",
            "required":true,

            "schema":{
              "type":"integer"
            }
          }
        ],

        "responses":{
          "200":{
            "description":"Booking found"
          },

          "404":{
            "description":"Booking not found"
          }
        }
      }
    },

    "/book":{

      "post":{

        "summary":"Create booking",

        "requestBody":{

          "required":true,

          "content":{

            "application/json":{

              "schema":{

                "type":"object",

                "properties":{

                  "id":{
                    "type":"integer"
                  },

                  "client":{
                    "type":"string"
                  },

                  "date":{
                    "type":"string"
                  }
                }
              },

              "example":{

                "id":1,
                "client":"Marta",
                "date":"25.05"
              }
            }
          }
        },

        "responses":{

          "200":{
            "description":"Booking created"
          },

          "400":{
            "description":"Booking failed"
          }
        }
      }
    },

    "/add-room":{

      "post":{

        "summary":"Add room",

        "requestBody":{

          "required":true,

          "content":{

            "application/json":{

              "schema":{

                "type":"object",

                "properties":{

                  "type":{
                    "type":"integer"
                  },

                  "price":{
                    "type":"number"
                  }
                }
              },

              "example":{

                "type":1,
                "price":2500
              }
            }
          }
        },

        "responses":{

          "200":{
            "description":"Room added"
          }
        }
      }
    },

    "/add-table":{

      "post":{

        "summary":"Add table",

        "requestBody":{

          "required":true,

          "content":{

            "application/json":{

              "schema":{

                "type":"object",

                "properties":{

                  "seats":{
                    "type":"integer"
                  },

                  "price":{
                    "type":"number"
                  }
                }
              },

              "example":{

                "seats":4,
                "price":1200
              }
            }
          }
        },

        "responses":{

          "200":{
            "description":"Table added"
          }
        }
      }
    },

    "/booking/{id}":{

      "put":{

        "summary":"Update booking",

        "parameters":[

          {
            "name":"id",
            "in":"path",
            "required":true,

            "schema":{
              "type":"integer"
            }
          }
        ],

        "requestBody":{

          "required":true,

          "content":{

            "application/json":{

              "schema":{

                "type":"object",

                "properties":{

                  "client":{
                    "type":"string"
                  },

                  "date":{
                    "type":"string"
                  }
                }
              },

              "example":{

                "client":"Updated Client",
                "date":"01.06"
              }
            }
          }
        },

        "responses":{

          "200":{
            "description":"Booking updated"
          }
        }
      },

      "delete":{

        "summary":"Delete booking",

        "parameters":[

          {
            "name":"id",
            "in":"path",
            "required":true,

            "schema":{
              "type":"integer"
            }
          }
        ],

        "responses":{

          "200":{
            "description":"Booking deleted"
          }
        }
      }
    }

    
  }
}

)";
  return res;
});


    app
    .port(18080)
    .multithreaded()
    .run();

    return 0;
}