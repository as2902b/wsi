var webservice = require('./models/webservice');

module.exports = function (app) {
    app.get('/services', function (req, res) {});
    app.post('/service/:id', function (req, res) {});
};
