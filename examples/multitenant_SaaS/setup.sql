create table if not exists notes (dateOf datetime, noteId bigint auto_increment primary key, userId bigint, note varchar(1000));
create table if not exists users (userId bigint auto_increment primary key, email varchar(100), hashed_pwd varchar(100), verified smallint, verify_token varchar(30), session varchar(100));
create unique index if not exists users1 on users (email);
