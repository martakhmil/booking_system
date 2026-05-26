import room1 from "./assets/room1.png";
import room2 from "./assets/room2.png";
import room3 from "./assets/room3.png";
import suite from "./assets/suite.png";
import restaurant from "./assets/restaurant.png";

import {useEffect,useState} from "react";

import{
LayoutDashboard,
Hotel,
Calendar,
PlusCircle,
Trash2,
ArrowLeft
}from "lucide-react";

const API="https://bookingsystem-production-7556.up.railway.app";

const resourceImages=[
room1,
room2,
room3,
suite
];

function App(){

const [role,setRole]=useState("");
const [section,setSection]=useState("dashboard");

const [resources,setResources]=useState([]);
const [bookings,setBookings]=useState([]);

const [resourceId,setResourceId]=useState("");
const [client,setClient]=useState("");
const [date,setDate]=useState("");

const [roomType,setRoomType]=useState(1);
const [roomPrice,setRoomPrice]=useState("");

const [tableSeats,setTableSeats]=useState("");
const [tablePrice,setTablePrice]=useState("");


const [editingBooking,setEditingBooking]=useState(null);
const [editingResource,setEditingResource]=useState(null);

const [editClient,setEditClient]=useState("");
const [editDate,setEditDate]=useState("");

const [editPrice,setEditPrice]=useState("");
const [editType,setEditType]=useState(1);
const [editSeats,setEditSeats]=useState("");
const [editingIsTable,setEditingIsTable]=useState(false);
// Конвертує 2026-05-26 -> 26.05
const formatDateToBackend = (dateStr) => {
  if (!dateStr) return "";
  const parts = dateStr.split("-");
  if (parts.length !== 3) return dateStr;
  return `${parts[2]}.${parts[1]}`;
};

// Конвертує 26.05 -> 2026-05-26 (підставляє поточний рік)
const formatDateToFrontend = (dateStr) => {
  if (!dateStr || !dateStr.includes(".")) return "";
  const parts = dateStr.split(".");
  const currentYear = new Date().getFullYear(); // 2026
  return `${currentYear}-${parts[1]}-${parts[0]}`;
};
async function loadResources(){

const res=await fetch(`${API}/resources`);
const data=await res.json();

const arr=Object.values(data || {}).map((x,index)=>({

...x,

image:
x.info.toLowerCase().includes("столик")
?restaurant
:resourceImages[index%4]

}));

setResources(arr);
}

async function loadBookings(){

const res=await fetch(`${API}/bookings`);
const data=await res.json();

setBookings(Object.values(data || {}));
}

useEffect(()=>{

loadResources();
loadBookings();

},[]);

async function bookResource(){

await fetch(`${API}/book`,{

method:"POST",

headers:{"Content-Type":"application/json"},

body:JSON.stringify({

id:Number(resourceId),
client,
date: formatDateToBackend(date)

})

});

setResourceId("");
setClient("");
setDate("");

loadResources();
loadBookings();
}

async function addRoom(){
  const priceNum = Number(roomPrice);

  // Перевірка перед відправкою, щоб знати, чи не пусте поле
  if (isNaN(priceNum) || priceNum <= 0) {
    alert("Будь ласка, введіть коректну ціну номера (більшу за 0)");
    return;
  }

  const response = await fetch(`${API}/add-room`,{
    method:"POST",
    headers:{ "Content-Type":"application/json" },
    body:JSON.stringify({
      type: Number(roomType),
      price: priceNum
    })
  });

  if (response.ok) {
    setRoomPrice("");
    setRoomType(1);
    loadResources();
    alert("Кімнату успішно додано!");
  } else {
    alert("Сервер відхилив додавання кімнати");
  }
}

async function addTable(){
  const seatsNum = Number(tableSeats);
  const priceNum = Number(tablePrice);

  if (isNaN(seatsNum) || seatsNum <= 0 || !Number.isInteger(seatsNum)) {
    alert("Будь ласка, введіть коректну кількість місць (ціле число більше 0)");
    return;
  }

  if (isNaN(priceNum) || priceNum <= 0) {
    alert("Будь ласка, введіть коректну ціну столика (більшу за 0)");
    return;
  }

  const response = await fetch(`${API}/add-table`,{
    method:"POST",
    headers:{ "Content-Type":"application/json" },
    body:JSON.stringify({
      seats: seatsNum,
      price: priceNum
    })
  });

  if (response.ok) {
    setTableSeats("");
    setTablePrice("");
    loadResources();
    alert("Столик успішно додано!");
  } else {
    alert("Сервер відхилив додавання столика");
  }
}

async function deleteResource(id){

await fetch(`${API}/resources/${id}`,{

method:"DELETE"

});

loadResources();
loadBookings();
}

async function deleteBooking(id){
  await fetch(`${API}/booking/${id}`,{ method:"DELETE" });
  loadBookings();
  loadResources();
}

async function updateBooking(id){
  await fetch(`${API}/booking/${id}`,{
    method:"PUT",
    headers: { "Content-Type":"application/json" },
    body:JSON.stringify({
      client: editClient,
      date: formatDateToBackend(editDate) // Конвертуємо перед збереженням
    })
  });
  setEditingBooking(null);
  loadBookings();
  loadResources();
}
async function updateResource(id){
  const priceNum = Number(editPrice);
  
  if (isNaN(priceNum) || priceNum <= 0) {
    alert("Будь ласка, введіть коректну ціну (більшу за 0)");
    return;
  }

  if (editingIsTable) {
    const seatsNum = Number(editSeats);
    if (isNaN(seatsNum) || seatsNum <= 0 || !Number.isInteger(seatsNum)) {
      alert("Будь ласка, введіть коректну кількість місць (ціле число більше за 0)");
      return;
    }
  }

  const payload = editingIsTable 
    ? { price: priceNum, seats: Number(editSeats) }
    : { price: priceNum, type: Number(editType) };

  // Додано збереження відповіді для обробки помилок сервера
  const response = await fetch(`${API}/resources/${id}`, {
    method: "PUT",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(payload)
  });

  if (!response.ok) {
    alert("Помилка оновлення ресурсу на сервері");
    return;
  }

  setEditingResource(null);
  setEditingIsTable(false);
  setEditSeats("");
  setEditPrice("");
  loadResources();
}

const freeResources=
resources.filter(x=>x.free).length;

const roomCount=
resources.filter(x=>
x.info.toLowerCase().includes("номер")
).length;

const tableCount=
resources.filter(x=>
x.info.toLowerCase().includes("столик")
).length;

return(

<div style={appStyle}>

{!role&&(

<div style={landingContainer}>

<div style={overlay}></div>

<div style={landingCard}>

<h1 style={mainTitle}>
BOOKING
</h1>

<div style={roleButtons}>

<button
style={primaryButton}
onClick={()=>{
setRole("user");
setSection("resources");
}}
>
User Panel
</button>

<button
style={dangerButton}
onClick={()=>{
setRole("admin");
setSection("dashboard");
}}
>
Admin Panel
</button>

</div>

</div>

</div>
)}

{role&&(

<div style={dashboardLayout}>

<div style={sidebar}>

<div>

<h2 style={logo}>
BOOKING
</h2>

<div
style={
section==="dashboard"
?activeSidebarItem
:sidebarItem
}
onClick={()=>setSection("dashboard")}
>
<LayoutDashboard size={18}/>
Dashboard
</div>

<div
style={
section==="resources"
?activeSidebarItem
:sidebarItem
}
onClick={()=>setSection("resources")}
>
<Hotel size={18}/>
Resources
</div>

{role==="user"&&(

<div
style={
section==="book"
?activeSidebarItem
:sidebarItem
}
onClick={()=>setSection("book")}
>
<PlusCircle size={18}/>
Book
</div>

)}

<div
style={
section==="bookings"
?activeSidebarItem
:sidebarItem
}
onClick={()=>setSection("bookings")}
>
<Calendar size={18}/>
Bookings
</div>

{role==="admin"&&(

<>

<div
style={
section==="addRoom"
?activeSidebarItem
:sidebarItem
}
onClick={()=>setSection("addRoom")}
>
<PlusCircle size={18}/>
Add Room
</div>

<div
style={
section==="addTable"
?activeSidebarItem
:sidebarItem
}
onClick={()=>setSection("addTable")}
>
<PlusCircle size={18}/>
Add Table
</div>

</>

)}

</div>

<button
style={backButton}
onClick={()=>{
setRole("");
}}
>
<ArrowLeft size={18}/>
Back
</button>

</div>

<div style={content}>

<div style={topbar}>

<h1 style={dashboardTitle}>
{role==="admin"
?"Admin Dashboard"
:"Booking Dashboard"}
</h1>



</div>

{section==="dashboard"&&(

<div style={statsGrid}>

<div style={statsCard}>
<h2>{resources.length}</h2>
<p>Total Resources</p>
</div>

<div style={statsCard}>
<h2>{bookings.length}</h2>
<p>Total Bookings</p>
</div>

<div style={statsCard}>
<h2>{freeResources}</h2>
<p>Available Resources</p>
</div>

<div style={statsCard}>
<h2>{roomCount}</h2>
<p>Rooms</p>
</div>

<div style={statsCard}>
<h2>{tableCount}</h2>
<p>Restaurant Tables</p>
</div>

</div>
)}

{section==="resources"&&(

<>

<h2 style={sectionTitle}>
Resources
</h2>

<div style={gridStyle}>

{resources.map(x=>(

<div
key={x.id}
style={cardStyle}

onMouseEnter={(e)=>{
e.currentTarget.style.transform=
"translateY(-10px)";
}}

onMouseLeave={(e)=>{
e.currentTarget.style.transform=
"translateY(0px)";
}}
>

<img
src={x.image}
style={imageStyle}
/>

<h2>
{x.info}
</h2>

<p style={{
marginTop:"12px",
fontWeight:"bold",
color:x.free
?"#ffffff"
:"#8b8b8b"
}}>
{x.free
?"Available"
:"Booked"}
</p>

{!x.free&&role==="user"&&(
<p style={{
marginTop:"10px",
color:"#8b8b8b"
}}>
Unavailable for booking
</p>
)}

<p style={{
marginTop:"10px",
opacity:"0.7"
}}>
ID: {x.id}
</p>

{role==="admin"&&(

<div style={{
display:"flex",
gap:"12px",
marginTop:"18px"
}}>

<button
  style={primaryButton}
  onClick={() => {
    setEditingResource(x.id);
    
    // Безпечний парсинг ціни (якщо не знайшло — ставимо "0")
    const priceMatch = x.info.match(/ціна=(\d+(\.\d+)?)/);
    setEditPrice(priceMatch ? priceMatch[1] : "0");

    // Визначаємо тип кімнати
    if (x.info.toLowerCase().includes("люкс")) {
      setEditType(2);
    } else {
      setEditType(1);
    }

    // Перевіряємо, чи це столик
    const isTable = x.info.toLowerCase().includes("столик");
    setEditingIsTable(isTable);

    // Безпечний парсинг місць для столика
    if (isTable) {
      const match = x.info.match(/місця=(\d+)/);
      setEditSeats(match ? match[1] : "0");
    } else {
      setEditSeats(""); // Для кімнат залишаємо порожнім
    }
  }}
>
  Edit
</button>

<button
style={deleteButton}
onClick={()=>
deleteResource(x.id)
}
>
<Trash2 size={16}/>
Delete
</button>

</div>

)}

</div>

))}

</div>

</>
)}

{section==="book"&&role==="user"&&(

<>

<h2 style={sectionTitle}>
Book Resource
</h2>

<div style={formCard}>

<input
placeholder="Resource ID"
value={resourceId}
onChange={(e)=>
setResourceId(e.target.value)
}
style={inputStyle}
/>

<input
placeholder="Client Name"
value={client}
onChange={(e)=>
setClient(e.target.value)
}
style={inputStyle}
/>

<input
type="date"
value={date}
onChange={(e)=>
setDate(e.target.value)
}
style={{
...inputStyle,
colorScheme:"dark",
paddingRight:"10px"
}}
/>

<button
style={primaryButton}
onClick={bookResource}
>
Book Now
</button>

</div>

</>
)}

{section==="addRoom"&&role==="admin"&&(

<div style={formCard}>

<select
  value={roomType}
  onChange={(e) => setRoomType(Number(e.target.value))} // Додано Number()
  style={inputStyle}
>
  <option value={1}>STANDARD</option>
  <option value={2}>LUX</option>
</select>

<input
  type="number" // Захист від введення букв
  min="0.01"
  step="0.01"
  placeholder="Price"
  value={roomPrice}
  onChange={(e)=> setRoomPrice(e.target.value)}
  style={inputStyle}
/>

<button
style={primaryButton}
onClick={addRoom}
>
Add Room
</button>

</div>
)}

{section==="addTable"&&role==="admin"&&(

<div style={formCard}>

<input
  type="number" // Захист
  min="1"
  step="1"
  placeholder="Seats"
  value={tableSeats}
  onChange={(e)=> setTableSeats(e.target.value)}
  style={inputStyle}
/>

<input
  type="number" // Захист
  min="0.01"
  step="0.01"
  placeholder="Price"
  value={tablePrice}
  onChange={(e)=> setTablePrice(e.target.value)}
  style={inputStyle}
/>

<button
style={primaryButton}
onClick={addTable}
>
Add Table
</button>

</div>
)}

{section==="bookings"&&(

<div style={gridStyle}>

{bookings.map(x=>(

<div
key={x.bookingId}
style={cardStyle}
>

<h2>
Booking #{x.bookingId}
</h2>

<p>
Resource ID: {x.resourceId}
</p>

<p>
Client: {x.client}
</p>

<p>
Date: {x.date}
</p>

{role==="admin"&&(

<div style={{
display:"flex",
gap:"12px",
marginTop:"18px"
}}>

<button
style={primaryButton}
onClick={() => {
  setEditingBooking(x.bookingId);
  setEditClient(x.client);
  setEditDate(formatDateToFrontend(x.date)); // Конвертуємо назад для інпута календаря
}}
>
Edit
</button>

<button
style={deleteButton}
onClick={()=>
deleteBooking(x.bookingId)
}
>
Cancel Booking
</button>

</div>

)}

</div>

))}

</div>
)}

</div>

</div>
)}

{editingBooking&&(

<div style={modalOverlay}>

<div style={modalStyle}>

<h2>Edit Booking</h2>

<input
value={editClient}
onChange={(e)=>
setEditClient(e.target.value)
}
style={inputStyle}
/>

<input
type="date"
value={editDate}
onChange={(e)=>
setEditDate(e.target.value)
}
style={{
...inputStyle,
colorScheme:"dark",
paddingRight:"10px"
}}
/>

<div style={{
display:"flex",
gap:"16px"
}}>

<button
style={primaryButton}
onClick={()=>
updateBooking(editingBooking)
}
>
Save
</button>

<button
style={dangerButton}
onClick={()=>
setEditingBooking(null)
}
>
Cancel
</button>

</div>

</div>

</div>
)}

{editingResource && (
  <div style={modalOverlay}>
    <div style={modalStyle}>
      <h2>Edit Resource</h2>

      {!editingIsTable && (
        <select
          value={editType}
          onChange={(e) => setEditType(Number(e.target.value))}
          style={inputStyle}
        >
          <option value={1}>STANDARD</option>
          <option value={2}>LUX</option>
        </select>
      )}

      {editingIsTable && (
        <input
          type="number" // Захист на рівні браузера
          min="1"
          step="1"
          placeholder="Seats"
          value={editSeats}
          onChange={(e) => setEditSeats(e.target.value)}
          style={inputStyle}
        />
      )}

      <input
        type="number" // Захист на рівні браузера
        min="0.01"
        step="0.01"
        placeholder="New Price"
        value={editPrice}
        onChange={(e) => setEditPrice(e.target.value)}
        style={inputStyle}
      />

      <div style={{ display: "flex", gap: "16px" }}>
        <button
          style={primaryButton}
          onClick={() => updateResource(editingResource)}
        >
          Save
        </button>

        <button
          style={dangerButton}
          onClick={() => {
            setEditingResource(null);
            setEditingIsTable(false);
          }}
        >
          Cancel
        </button>
      </div>
    </div>
  </div>
)}

</div>

);
}

const appStyle={
minHeight:"100vh",
background:"#000000",
color:"white",
fontFamily:"Arial"
};

const landingCard={
position:"relative",
zIndex:"2",
background:"rgba(10,10,10,0.82)",
padding:"70px",
borderRadius:"34px",
backdropFilter:"blur(16px)",
textAlign:"center",
width:"520px",
border:"1px solid rgba(255,255,255,0.08)",
boxShadow:"0 0 40px rgba(255,255,255,0.06)"
};

const sidebar={
width:"260px",
minHeight:"100vh",
background:"#050505",
padding:"30px",
display:"flex",
flexDirection:"column",
justifyContent:"space-between",
borderRight:"1px solid rgba(255,255,255,0.08)"
};

const sidebarItem={
display:"flex",
alignItems:"center",
gap:"12px",
padding:"16px",
borderRadius:"16px",
cursor:"pointer",
marginBottom:"14px",
transition:"0.3s",
color:"white"
};

const activeSidebarItem={
...sidebarItem,
background:"#ffffff",
color:"#000000",
fontWeight:"bold"
};

const statsCard={
background:"#0b0b0b",
padding:"34px",
borderRadius:"28px",
border:"1px solid rgba(255,255,255,0.08)",
boxShadow:"0 0 25px rgba(255,255,255,0.03)"
};

const cardStyle={
background:"#0b0b0b",
padding:"20px",
borderRadius:"28px",
transition:"0.4s",
cursor:"pointer",
border:"1px solid rgba(255,255,255,0.08)",
boxShadow:"0 0 25px rgba(255,255,255,0.03)"
};

const formCard={
display:"flex",
flexWrap:"wrap",
gap:"18px",
background:"#0b0b0b",
padding:"24px",
borderRadius:"28px",
border:"1px solid rgba(255,255,255,0.08)"
};

const inputStyle={
padding:"16px",
borderRadius:"16px",
border:"1px solid rgba(255,255,255,0.08)",
background:"#050505",
color:"white",
minWidth:"220px",
outline:"none",
fontSize:"16px"
};

const primaryButton={
padding:"16px 24px",
border:"1px solid white",
borderRadius:"16px",
background:"#ffffff",
color:"#000000",
cursor:"pointer",
fontWeight:"bold",
transition:"0.3s"
};

const dangerButton={
padding:"16px 24px",
border:"1px solid white",
borderRadius:"16px",
background:"#000000",
color:"#ffffff",
cursor:"pointer",
fontWeight:"bold",
transition:"0.3s"
};

const deleteButton={
marginTop:"0px",
padding:"12px 18px",
border:"1px solid rgba(255,255,255,0.2)",
borderRadius:"14px",
background:"#111111",
color:"white",
cursor:"pointer"
};

const backButton={
padding:"16px",
border:"1px solid rgba(255,255,255,0.08)",
borderRadius:"16px",
background:"#0b0b0b",
color:"white",
cursor:"pointer",
display:"flex",
alignItems:"center",
gap:"8px"
};

const mainTitle={
fontSize:"74px",
marginBottom:"10px",
letterSpacing:"3px",
color:"white"
};

const dashboardTitle={
fontSize:"52px",
color:"white"
};

const logo={
fontSize:"38px",
marginBottom:"40px",
color:"white"
};

const dashboardLayout={
display:"flex"
};

const content={
flex:1,
padding:"40px",
background:"#000000"
};

const topbar={
marginBottom:"40px"
};

const statsGrid={
display:"grid",
gridTemplateColumns:"repeat(auto-fit,minmax(220px,1fr))",
gap:"20px"
};

const gridStyle={
display:"grid",
gridTemplateColumns:"repeat(auto-fit,minmax(300px,1fr))",
gap:"24px"
};

const imageStyle={
width:"100%",
height:"220px",
objectFit:"cover",
borderRadius:"18px",
marginBottom:"18px"
};

const sectionTitle={
fontSize:"34px",
marginBottom:"24px",
color:"white"
};

const roleButtons={
display:"flex",
justifyContent:"center",
gap:"20px",
marginTop:"30px"
};

const landingContainer={
minHeight:"100vh",
display:"flex",
justifyContent:"center",
alignItems:"center",
backgroundImage:"url('https://images.unsplash.com/photo-1566073771259-6a8506099945?q=80&w=2070&auto=format&fit=crop')",
backgroundSize:"cover",
backgroundPosition:"center",
position:"relative"
};

const overlay={
position:"absolute",
inset:"0",
background:"rgba(0,0,0,0.72)"
};

const modalOverlay={
position:"fixed",
inset:"0",
background:"rgba(0,0,0,0.75)",
display:"flex",
justifyContent:"center",
alignItems:"center",
zIndex:"1000"
};

const modalStyle={
background:"#0b0b0b",
padding:"40px",
borderRadius:"28px",
display:"flex",
flexDirection:"column",
gap:"18px",
minWidth:"420px",
border:"1px solid rgba(255,255,255,0.08)"
};

export default App;