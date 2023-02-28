create table if not exists customers (firstName varchar(30), lastName varchar(30), customerID bigserial primary key);
create table if not exists items (name varchar(30), description varchar(200), itemID bigserial primary key);
create table if not exists orders (customerID bigint, orderID bigserial primary key);
create table if not exists orderItems (orderID bigint, itemID bigint, quantity bigint);
