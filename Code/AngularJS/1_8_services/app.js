(function (){
    var app = angular.module('mon_module', []);
   
    app.directive('linkList', ['$http', '$log', function ($http, $log) {
        return {
            restrict: 'E',
            templateUrl: 'link_list.html',
            controller: function () {
                this.data = {};
                var obj = this;
                $log.warn('OK');
                $http.get('data.json').
                    success(function (info) {
                        $log.debug('JSON loaded');
                        obj.data = info;
                    }).
                    error(function () {
                        $log.error('Chargement impossible');
                    });
                this.currentInfo = '';

                this.addInfo = function () {
                    this.data.info += ' ' + this.currentInfo;
                    this.currentInfo = '';
                    this.data.msg = '';
                };
            },
            controllerAs: 'ctrl'
        };
    }]);
})();
