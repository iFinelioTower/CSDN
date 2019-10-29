PATH=$PATH:/home/luwenjiang/3128linux/buildroot/output/rockchip_rk3128_game/host/bin/arm-buildroot-linux-gnueabihf-gcc
luwenjiang@sunchip2:~/databack/Tower_workspace/RK3128_Linux4.4/sqlite-autoconf-3300100$ ./configure --host=arm-buildroot-linux-gnueabihf --prefix=/home/luwenjiang/databack/Tower_workspace/RK3128_Linux4.4/sqlite-autoconf-3300100/build

原语:
CREATE TABLE
INSERT INTO
select * from

C代码:
sqlite3_initialize();

编写测试程序:
gcc sqlite_test.c -lsqlite3 -L./build/lib/ -I./build/include/ -I./include