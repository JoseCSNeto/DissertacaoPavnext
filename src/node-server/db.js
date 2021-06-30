const Pool = require("pg").Pool;

const pool = new Pool({
    user: "",
    password: "",
    host: "",
    database: "",
    port: 5432,
    connectionString: process.env.DATABASE_CONNECTION, // <--- not defined without .env file definition
    ssl: {
      rejectUnauthorized: false,
    }
});

pool
.connect()
.then(() => console.log("connected to database"))
.catch((err) => console.error(err));

module.exports = pool;