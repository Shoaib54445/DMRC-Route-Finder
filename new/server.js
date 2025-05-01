const express = require("express");
const cors = require("cors");
const fs = require("fs");
const { execFile } = require("child_process");
const path = require("path");

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(express.urlencoded({ extended: true })); 
app.use(express.json());
app.use(express.static(path.join(__dirname, "public")));


app.use((req, res, next) => {
  res.set('Cache-Control', 'no-store');
  next();
});


app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"));
});


app.post("/route", (req, res) => {
  console.log("Request received");
  const { source, destination, mode } = req.body;


  // console.log(req.body);
  // console.log(source, destination, mode);



  if (!source || !destination || !mode) {
    return res.status(400).json({ error: "Missing required fields" });
  }

  console.log("Received request:", source, destination, mode);

  const exePath = path.join(__dirname, "dmrc.exe");

  execFile(exePath, [source, destination, mode], (error, stdout, stderr) => {
    if (error) {
      console.error("Execution Error:", error);
      console.error("stderr:", stderr);
      return res.status(500).json({ error: "Execution failed" });
    }

    const outputPath = path.join(__dirname, "output.json");

    fs.readFile(outputPath, "utf8", (err, data) => {
      if (err) {
        console.error("Read Error:", err);
        return res.status(500).json({ error: "Output read failed" });
      }

      res.setHeader("Content-Type", "application/json");
      res.send(data);
    });
  });
});

// Start server
app.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}`);
});
