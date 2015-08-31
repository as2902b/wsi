var mongoose = require('mongoose');

var kvpSchema = ({key: String, value: String});
var kvpArraySchema = ({members : [kvpSchema]});

var authSchema = ({
    type: String,
    credentials : [kvpArraySchema]
});

var capabilitySchema = ({
    detail  :   [kvpArraySchema],
    params  :   [kvpArraySchema],
    headers :   [kvpArraySchema],
    response:   [kvpArraySchema],
    tags    :   [String]
});

var wsSchema = new mongoose.Schema({
    id:             String,
    name:           String,
    desc:           {type : String, default : "A Sample Service"},
    platforms :     [kvpArraySchema],
    auth :          authSchema,
    capability :    [capabilitySchema]
});

module.exports = mongoose.model('webservice', wsSchema);
