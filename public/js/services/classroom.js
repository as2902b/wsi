angular.module('wsiService', [])
    .factory('studentEntry', ['$http', function ($http) {
        return {
            get : function () {
                return $http.get('/get');
            },
            create : function (data) {
                return $http.post('/add', data);
            }
        };
    }]);
