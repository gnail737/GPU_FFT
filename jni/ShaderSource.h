
//every shader needs folllowing lines
#define GL3ES_VERSION_SEPC "#version 300 es        \n"
#define FP_PREC_SPEC   "precision mediump float;   \n"
#define FS_LAYOUT_LINE "layout(location = 0) out vec4 fragColor; \n"
#define L_(STMT) "  " #STMT "  \n"

static const char VERTEX_SHADER[] =
	GL3ES_VERSION_SEPC
    L_(layout(location = 0) in vec2 vPosition;     	)
	L_(void main() {                               	)
	L_(  gl_Position = vec4(vPosition, 0.0, 1.0);	)
	L_(}  											);

/**
static const char FRAGMENT_SHADER[] =
    GL3ES_VERSION_SEPC
    FP_PREC_SPEC
    L_(layout(location = 0) out vec4 fragColor;		)
    L_(void main() { 								)
   	L_(  fragColor = vec4(0.95, 0.95, 0, 0); 		)
    L_(} 											);

static const char VERTEX_FILL_SHADER[] =
	GL3ES_VERSION_SEPC
    L_(layout(location = 0) in vec4 vPosition;			)
	L_(out vec2 texCoord; 								)
	L_(void main() { 									)
	L_(  gl_Position = vec4(vPosition.xy, 0.0, 1.0);	)
	L_(  texCoord = vPosition.zw; 						)
	L_(} 												);
**/

/*
 * void reverseInPlace(UINT * input, UINT m)
{
	UINT out = 0, scrape = 0, k;
	for (k=m; k>=1; k--) {
		//right shift till see the last bit
		scrape =(*input)>>(k-1) & 0x00000001;
		//out is assembled bit by bit from last to first
	    out += ((scrape == 0) ?  0 : (1<<(m-k)));
	}

	*input = out;
}
 */
static const char FRAGMENT_SHADER[] =
	GL3ES_VERSION_SEPC
	FP_PREC_SPEC
    FS_LAYOUT_LINE
    L_(uniform sampler2D floatArray;											)
    L_(uniform int nbit;														)
    //L_(uniform sampler2D permArray;											)
    L_(int reverseBits(int inputNum, int numBits) {              				)
    L_(  int outNum = 0;														)
    L_(  for (int i=numBits; i>=1; i--) {										)
    L_(    outNum += (((inputNum>>(i-1)) & 1) * (1<<(numBits-i)));              )
    L_(  }   																	)
    L_(  return outNum; 														)
    L_(}																		)
    L_(void main() { 															)
    L_(  ivec2 lookupInd = ivec2(gl_FragCoord.xy);                      		)
    L_(  lookupInd.x = reverseBits(lookupInd.x, nbit);					   		)
    L_(  fragColor = vec4(texelFetch(floatArray, lookupInd, 0).rg, 0.0, 1.0);	)
    //L_(  fragColor = vec4(texture(floatArray, texCoord).rgb, 1.0); 			)
    L_(}                                                             			);

/* for simplicity first assume we only use 1D Texture arrays, so GL_FragCoord only care about one dimensional data */
static const char DECIMATION_IN_TIME_SHADER[] =
	GL3ES_VERSION_SEPC
	FP_PREC_SPEC
	FS_LAYOUT_LINE
	L_(uniform sampler2D floatArray; 															)
    L_(uniform int butterflyIndex;              												)
    L_(const float PI = 3.141592654;															)
    L_(vec2 mult_twiddleFactor(vec2 inVec, float kn, float sum) {								)
    L_(  vec2 outVec = vec2(0);																	)
    L_(  outVec.x = inVec.x*cos(-2.0*PI*kn/sum) - inVec.y*sin(-2.0*PI*kn/sum);					)
    L_(  outVec.y = inVec.y*cos(-2.0*PI*kn/sum) + inVec.x*sin(-2.0*PI*kn/sum);					)
    L_(  return outVec;  																		)
    L_(} 																						)
	L_(void main() {																			)
	L_(  ivec2 lookupInd = ivec2(gl_FragCoord.xy);   											)
	L_(  ivec2 lookupInd_c = lookupInd ^ butterflyIndex;										)
	L_(  int signess = 1 - ((lookupInd.x & butterflyIndex) / butterflyIndex) * 2;           	)
	L_(  vec2 complex_pair_1 = vec2(signess) * texelFetch(floatArray, lookupInd, 0).rg;		)
	L_(  vec2 complex_pair_2 = texelFetch(floatArray, lookupInd_c, 0).rg;						)
	L_(  complex_pair_1 += complex_pair_2;        												)
	L_(  if (((butterflyIndex << 1) & lookupInd.x) != 0) {										)
	L_(    complex_pair_1 = mult_twiddleFactor(complex_pair_1, float(lookupInd.x&((butterflyIndex<<1)-1)), float(butterflyIndex<<2)); 	)
	L_(  })
	L_(  fragColor = vec4(complex_pair_1, 0.0, 1.0);											)
	L_(}																						);

/*
 * 			realTemp[k] = ( k < (k^magicMask) ? 1.f :-1.f) * realArray[k] + realArray[k^magicMask];
			imgTemp[k] = ( k < (k^magicMask) ? 1.f :-1.f) * imagineArray[k] + imagineArray[k^magicMask];
 *
 */
static const char POINT_SPRITE_VERTEX_SHADER[] =
		GL3ES_VERSION_SEPC
		"layout(location = 0) in vec2 Position; \n"
		//vertex shader for our point sprite model space is (-Width, Width)  x (-Height, Height)
		//we have to convert them to x range (-1, 1) before passing down to GL_POSITION
		//Thus multipler/scaleFactor is (1/Width, 1/Height)
		"const vec2 scaleFactor = vec2(1.0/720.0, 1.0/1038.0);            \n"
		"uniform sampler2D VPTexture;             \n"
		"uniform float PointSize;                 \n"
		"out vec2 velocity;                       \n"
		"void main() {                            \n"
		"   ivec2 lookupInd = ivec2(Position.x, 0); \n"
		//use look up to find the current position of point sprite then do transform to NDC (-1, 1)
		"   vec4 spritePosVelocity = texelFetch(VPTexture, lookupInd, 0); \n"
		"   vec2 spritePos = spritePosVelocity.zw*scaleFactor;            \n"
		"   gl_Position = vec4( spritePos , 0.0, 1.0 );                   \n"
		"   gl_PointSize = PointSize;                                     \n"
		"   velocity = spritePosVelocity.xy;                              \n"
		"}                                                                \n";

static const char POINT_SPRITE_FRAGMENT_SHADER[] =
		GL3ES_VERSION_SEPC
		FP_PREC_SPEC
		FS_LAYOUT_LINE
		"in vec2 velocity;                \n"
        "uniform sampler2D spriteImage;   \n"
		"void main() {                    \n"
		//we scale velocity to normal range before displayig
		"   vec2 absVelocity = abs(velocity)/length(velocity); \n"
		"   fragColor = vec4(absVelocity, 0.0, 0.95);           \n"
        "}                                                     \n";

static const char UPDATE_VELOCITY_FRAGMENT_SHADER[] =
		GL3ES_VERSION_SEPC
	    FP_PREC_SPEC
	    FS_LAYOUT_LINE
	    "uniform sampler2D VelocityTexture;                                            \n"
	    "uniform float deltaTime;                                                      \n"
	    //pixel offset for boundary
	    "const float boundaryWidth = 12.0;                                               \n"
	    //bundary position in model space (-width, width) (-height, height) here only pass positive values
	    "const vec2  boundaryPosition = vec2(720.0, 1038.0);                         \n"
	    "void main()  {                                                                \n"
	    "   ivec2 fragCoord = ivec2(floor(gl_FragCoord.x), 0);                         \n"
	    "   vec2 acceleration = vec2(sin(gl_FragCoord.x/16.0), cos(gl_FragCoord.x/16.0));  \n"
	    "   vec4 oldVal = texelFetch(VelocityTexture, fragCoord, 0);                   \n"
	    "   vec2 oldVelocity = oldVal.xy;                                              \n"
	    "   vec2 oldPosition = oldVal.zw;                                              \n"
	    //update using Newton's Law
	    "   vec2 newVelocity = deltaTime * acceleration + oldVelocity;                 \n"
	    "   vec2 newPosition = deltaTime * oldVelocity + oldPosition;                  \n"
	    //doing collision detection with boundary test
	    "if (newPosition.x > boundaryPosition.x - boundaryWidth || newPosition.x < -boundaryPosition.x + boundaryWidth) {   \n"
	    "   newPosition = oldPosition;  newVelocity.x = -newVelocity.x;                \n"
	    "}                                                                             \n"
	    "if (newPosition.y > boundaryPosition.y - boundaryWidth || newPosition.y < -boundaryPosition.y + boundaryWidth) {   \n"
	    "   newPosition = oldPosition;  newVelocity.y = -newVelocity.y;                \n"
	    "}                                                                             \n"
	    "fragColor = vec4(newVelocity, newPosition);                                   \n"
	    "}                                                                             \n";

