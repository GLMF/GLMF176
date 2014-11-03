var http = require('http');
var sqlite3 = require('sqlite3').verbose();
var fs = require('fs');
var url = require('url');

var db = new sqlite3.Database('logs.db');

if (!fs.existsSync('logs.db'))
{
    db.run('create table logs (timestamp datetime primary key, msg text)');
};

var app = http.createServer(function (req, res) {
    var stmt;
    var data = '';
    var queryData = url.parse(req.url, true).query;

    res.setHeader('Content-Type', 'application/json');
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET');

    if (queryData.display !== undefined)
    {
        stmt = 'select * from logs';
        db.all(stmt, function (err, rows) {
            data = rows;
            res.end(JSON.stringify(data));
        });
    }
    else if (queryData.add !== undefined)
    {
        var timestamp = new Date().getTime();
        stmt = db.prepare('insert into logs (timestamp, msg) values(?, ?)');
        stmt.run(timestamp, queryData.msg);
        stmt.finalize();
        res.end(JSON.stringify({ time: timestamp }));
    };
});

app.listen(3000);
process.on('SIGINT', function () { db.close(); });
process.on('SIGTERM', function () { db.close(); });
