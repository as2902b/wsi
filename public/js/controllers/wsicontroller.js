angular.module('wsiController', [])
    .controller('mainController', ['$scope','$http','studentEntry', function($scope, $http, studentEntry) {
        $scope.formData = {};
        studentEntry.get()
            .success(function(data) {
                $scope.studentStatus = data;
            });
        $scope.createStudentEntry = function() {

            if ($scope.formData.sid != undefined) {
                studentEntry.create($scope.formData)
                    .success(function(data) {
                        $scope.formData = {};
                        $scope.studentStatus = data;
                    });
            }
        };
    }]);
