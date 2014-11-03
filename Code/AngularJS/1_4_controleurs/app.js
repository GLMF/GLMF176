(function (){
    var data = { name : 'GLMF', number : 176 };
    var app = angular.module('mon_module', []);

    app.controller('dataController', function () {
        this.data = data;
    });
})();
