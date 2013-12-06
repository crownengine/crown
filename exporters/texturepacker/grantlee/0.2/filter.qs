heightRel = 0;
 
var SetHeightRel = function(input)
{
    heightRel = input;
    return "";
};
SetHeightRel.filterName = "setHeightRel";
Library.addFilter("SetHeightRel");
 
var TransformY = function(input)
{
    return String(1.0 - input - heightRel);
};
TransformY.filterName = "transformY";
Library.addFilter("TransformY");