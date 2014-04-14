height = 0;
spriteHeight = 0;
 
var SetHeight = function(input)
{
    height = input;
    return "";
};
SetHeight.filterName = "setHeight";
Library.addFilter("SetHeight");

var SetSpriteHeight = function(input)
{
	spriteHeight = input;
	return "";
};
SetSpriteHeight.filterName = "setSpriteHeight";
Library.addFilter("SetSpriteHeight")
 
var TransformY = function(input)
{
    return String(height - input - spriteHeight);
};
TransformY.filterName = "transformY";
Library.addFilter("TransformY");