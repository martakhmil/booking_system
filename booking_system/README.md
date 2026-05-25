docker build -t hotel-system . 

docker run -it -p 18080:18080 -v "C:\Users\Lenovo\Desktop\vsc\oop\booking_system:/app" hotel-system

docker ps 
ПЕРЕГЛЯНУТИ КОНТЕЙНЕРИ

docker ps -a
ПЕРЕГЛЯНУТИ ВСІ КОНТЕЙНЕРИ

docker stop CONTAINER_ID
ЗУПИНИТИ КОНТЕЙНЕР

docker rm CONTAINER_ID
ВИДАЛИТИ КОНТЕЙНЕР

docker images
ПЕРЕГЛЯНУТИ IMAGE

docker rmi hotel-system
ВИДАЛИТИ IMAGE

http://localhost:18080/swagger
SWAGGER

API

http://localhost:18080/resources
всі ресурси

http://localhost:18080/bookings
всі бронювання



ЯКЩО ПОРТ ЗАЙНЯТИЙ

docker stop CONTAINER_ID

або:

docker rm CONTAINER_ID


npm run dev
запустити фронт апп.жсх
