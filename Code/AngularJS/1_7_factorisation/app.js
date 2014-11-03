(function (){
    var data = { name : 'GLMF', number : 176, info : '', msg : '', array : [176, 177, 178, 179] };
    var app = angular.module('mon_module', []);

    app.directive('linkList', function () {
        return {
            restrict: 'E',
            templateUrl: 'link_list.html',
            controller: function () {
                this.data = data;
                this.currentInfo = '';

                this.addInfo = function () {
                    this.data.info += ' ' + this.currentInfo;
                    this.currentInfo = '';
                    this.data.msg = '';
                };
            },
            controllerAs: 'ctrl'
        };
    });
})();
