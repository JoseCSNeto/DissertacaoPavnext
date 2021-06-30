const express = require("express");
const app = express();
const cors = require("cors");
const pool = require("./db");

//middleware
app.use(cors());
app.use(express.json()); //req.body

//ROUTES//
const http = require('http')
const swaggerUi = require('swagger-ui-express')
const swaggerFile = require('./swagger_output.json')

// http.createServer(app).listen(3000)
// console.log("Listening at:// port:%s (HTTP)", 3000)

app.use('/doc', swaggerUi.serve, swaggerUi.setup(swaggerFile))


// app.get('/', (req, res) => {
//   res.send('hello world');
// });
//create a measurements


app.post("/measurements/multiple", async (req, res) => {
      /*
            Swagger Documentation:
            #swagger.method = 'post'
            #swagger.tags = ['Measurements']
            #swagger.description = 'Endpoint to upload sensor data.'
            #swagger.parameters['sensorData'] = {
                in: 'body',
                description: 'Measurement from installation',
                required: true,
                type: 'object'
            }
        */
  try {
    //console.log(req.body);
    const measurements = req.body;
    var newMeasurement;
      //console.log(measurements);
      newMeasurement = await pool.query(
        "INSERT INTO measurements (temperature,humidity,xaxis,yaxis,zaxis,power_g1,energy_g1,power_g2,energy_g2,speed) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)",
        [
          measurements.temperature,
          measurements.humidity,
          measurements.xaxis,
          measurements.yaxis,
          measurements.zaxis,
          measurements.power_g1,
          measurements.energy_g1,
          measurements.power_g1,
          measurements.energy_g2,
          measurements.speed
        ]
      );
    
    result_string = `Inserted measurements!`;
    //console.log(result_string);
    res.send(result_string);
  } catch (err) {
    //console.log(err.message);
  }
});


//get all measurements
app.get("/measurements", async (req, res) => {
      /*
            Swagger Documentation:
            #swagger.method = 'get'
            #swagger.tags = ['Measurements']
            #swagger.description = 'Returns all measurements.'
        */  
  try {
    const allTodos = await pool.query("SELECT * FROM measurements order by measurement_id");
    res.json(allTodos.rows);
    //console.log(allTodos.rows);
  } catch (err) {
    console.error(err.message);
  }
});


app.get("/measurements/:sensor", async (req, res) => {
      /*
            Swagger Documentation:
            #swagger.method = 'get'
            #swagger.tags = ['Measurements']
            #swagger.description = 'Gets data from specific sensor.'
            #swagger.parameters['sensorData'] = {
                in: 'body',
                description: 'Measurement from installation',
                required: true,
                type: 'object'
            }
        */
  var getSensor = [req.params.sensor];

  try {
    const allMeasurements = await pool.query("SELECT tstamp,"+ getSensor + " FROM measurements order by measurement_id");
    var rows = allMeasurements.rowCount;
    var i = 0;

    
    for (i; i < rows; i++){
      //console.log(allMeasurements.rows[i].tstamp.valueOf());
      allMeasurements.rows[i].tstamp = allMeasurements.rows[i].tstamp.valueOf();
    }

    var result = JSON.stringify(allMeasurements.rows);
    var res1 = result.replace(/\"tstamp\":/g, "");
    res1 = res1.replace(/\"/g, "");
    res1 = res1.replace(/:/g, "");
    res1 = res1.replace(new RegExp(getSensor, "g"), "");
    res1 = res1.replace(/}/g, "]");
    res1 = res1.replace(/{/g, "[");
    res1 = 'var ' + getSensor + ' = '  + res1;

    res.send(res1);
   // console.log(res1);
  } catch (err) {
    console.error(err.message);
  }
});

app.get("/measurements/today", async (req, res) => {
        /*
            Swagger Documentation:
            #swagger.method = 'get'
            #swagger.tags = ['Measurements']
            #swagger.description = 'Gets data from today.'
            #swagger.parameters['sensorData'] = {
                in: 'body',
                description: 'Measurement from today',
                required: true,
                type: 'object'
            }
        */
  try {
    const allMeasurements = await pool.query("SELECT * FROM measurements WHERE tstamp::date = CURRENT_DATE order by measurement_id");
    res.json(allMeasurements.rows);
    //console.log(allMeasurements.rows);
  } catch (err) {
    console.error(err.message);
  }
});


// app.get("/hello", async (req, res) => {
//   try {
//     const { id } = req.params;
//     const measurements = await pool.query("SELECT * FROM measurements WHERE measurement_id = $1", [
//       id
//     ]);

//     res.send("hello");
//   } catch (err) {
//     console.error(err.message);
//   }
// });

//get a measurement

app.get("/measurements/:id", async (req, res) => {
        /*
            Swagger Documentation:
            #swagger.method = 'get'
            #swagger.tags = ['Measurements']
            #swagger.description = 'Gets specific measurement.'
            #swagger.parameters['sensorData'] = {
                in: 'body',
                description: 'Gets specific measurement',
                required: true,
                type: 'object'
            }
        */
  try {
    const { id } = req.params;
    const measurements = await pool.query("SELECT * FROM measurements WHERE measurement_id = $1", [
      id
    ]);

    res.json(measurements.rows[0]);
  } catch (err) {
    console.error(err.message);
  }
});

//update a measurements


//delete a measurements

app.delete("/measurements/:id", async (req, res) => {
          /*
            Swagger Documentation:
            #swagger.method = 'delete'
            #swagger.tags = ['Measurements']
            #swagger.description = 'Deletes specific measurement.'
            #swagger.parameters['sensorData'] = {
                in: 'body',
                description: 'Deletes specific measurement',
                required: true,
                type: 'object'
            }
        */
  try {
    const { id } = req.params;
    const deleteTodo = await pool.query("DELETE FROM measurements WHERE measurement_id = $1", [
      id
    ]);
    res.json("Todo was deleted!");
  } catch (err) {
    console.log(err.message);
  }
});



const port = process.env.PORT || 5431;
app.listen(port, () => {
  console.log("server has started");
});