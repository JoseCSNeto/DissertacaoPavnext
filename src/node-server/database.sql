CREATE TABLE users
(
	user_id serial NOT NULL,
  username TEXT NOT NULL,
  email TEXT NOT NULL,
  password TEXT NOT NULL,
  CONSTRAINT users_pkey PRIMARY KEY (user_id)
); 

CREATE TABLE installation
(
	installation_id serial NOT NULL,
	owner_id integer NOT NULL,
  location TEXT NOT NULL,
  CONSTRAINT installation_pkey PRIMARY KEY (installation_id),
	CONSTRAINT fk_owner FOREIGN KEY(owner_id) REFERENCES users(user_id)
);


CREATE TABLE measurements
(
  measurement_id serial NOT NULL,
	installation_id integer NOT NULL,
  tstamp timestamp without time zone DEFAULT now(),
  temperature real,
  humidity real,
  speed real,
  xAxis real,
  yAxis real,
  zAxis real,
  power_G1 real,
  energy_G1 real,
  power_G2 real,
  energy_G2 real,
  CONSTRAINT measurements_pkey PRIMARY KEY (measurement_id),
	CONSTRAINT fk_installation FOREIGN KEY(installation_id) REFERENCES installation(installation_id)
);