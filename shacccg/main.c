/*
	Vita Development Suite Samples
*/

/*	Basic rendering sample using SceShaccCg for shader compilation.

	This sample shows how to use SceShaccCg to render two triangles (one
	to clear the screen, one animated).
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctrl.h>
#include <display.h>
#include <gxm.h>
#include <kernel/iofilemgr.h>
#include <kernel/modulemgr.h>
#include <libdbg.h>
#include <sceconst.h>
#include <shacccg.h>

unsigned int sceLibcHeapSize = 0x800000;

/*	Define the width and height to render at native resolution.
*/
#define DISPLAY_WIDTH				960
#define DISPLAY_HEIGHT				544
#define DISPLAY_STRIDE_IN_PIXELS	1024

/*	Define the libgxm color format to render to.  This should be kept in sync
	with the display format to use with the SceDisplay library.
*/
#define DISPLAY_COLOR_FORMAT		SCE_GXM_COLOR_FORMAT_A8B8G8R8
#define DISPLAY_PIXEL_FORMAT		SCE_DISPLAY_PIXELFORMAT_A8B8G8R8

/*	Define the number of back buffers to use with this sample.  Most applications
	should use a value of 2 (double buffering) or 3 (triple buffering).
*/
#define DISPLAY_BUFFER_COUNT		3

/*	Define the maximum number of queued swaps that the display queue will allow.
	This limits the number of frames that the CPU can get ahead of the GPU,
	and is independent of the actual number of back buffers.  The display
	queue will block during sceGxmDisplayQueueAddEntry if this number of swaps
	have already been queued.
*/
#define DISPLAY_MAX_PENDING_SWAPS	2

/*	Define the MSAA mode.  This can be changed to 4X or 2X.
*/
#define MSAA_MODE					SCE_GXM_MULTISAMPLE_NONE

/*	Set this macro to 1 to manually allocate the memblock for the render target.
*/
#define MANUALLY_ALLOCATE_RT_MEMBLOCK		0

// Helper macro to align a value
#define ALIGN(x, a)					(((x) + ((a) - 1)) & ~((a) - 1))

// Data structure for clear geometry
typedef struct ClearVertex {
	float x;
	float y;
} ClearVertex;

// Data structure for basic geometry
typedef struct BasicVertex {
	float x;
	float y;
	float z;
	uint32_t color;
} BasicVertex;

/*	Data structure to pass through the display queue.  This structure is
	serialized during sceGxmDisplayQueueAddEntry, and is used to pass
	arbitrary data to the display callback function, called from an internal
	thread once the back buffer is ready to be displayed.

	In this example, we only need to pass the base address of the buffer.
*/
typedef struct DisplayData
{
	void *address;
} DisplayData;

// Callback function for displaying a buffer
static void displayCallback(const void *callbackData);

// Callback function to allocate memory for the shader patcher
static void *patcherHostAlloc(void *userData, uint32_t size);

// Callback function to allocate memory for the shader patcher
static void patcherHostFree(void *userData, void *mem);

// Helper function to allocate memory and map it for the GPU
static void *graphicsAlloc(SceKernelMemBlockType type, uint32_t size, uint32_t alignment, uint32_t attribs, SceUID *uid);

// Helper function to free memory mapped to the GPU
static void graphicsFree(SceUID uid);

// Helper function to allocate memory and map it as vertex USSE code for the GPU
static void *vertexUsseAlloc(uint32_t size, SceUID *uid, uint32_t *usseOffset);

// Helper function to free memory mapped as vertex USSE code for the GPU
static void vertexUsseFree(SceUID uid);

// Helper function to allocate memory and map it as fragment USSE code for the GPU
static void *fragmentUsseAlloc(uint32_t size, SceUID *uid, uint32_t *usseOffset);

// Helper function to free memory mapped as fragment USSE code for the GPU
static void fragmentUsseFree(SceUID uid);

// Callback function to open file for shader compiler
static SceShaccCgSourceFile *openFileCallback(
	const char *fileName,
	const SceShaccCgSourceLocation *includedFrom,
	const SceShaccCgCompileOptions *compileOptions,
	void *userData,
	const char **errorString);

// Callback function to release file for shader compiler
static void releaseFileCallback(
	const SceShaccCgSourceFile *file,
	const SceShaccCgCompileOptions *compileOptions,
	void *userData);

// Mark variable as used
#define	UNUSED(a)					(void)(a)

// Entry point
int main(void)
{
	int err = SCE_OK;
	UNUSED(err);

	// set up parameters
	SceGxmInitializeParams initializeParams;
	memset(&initializeParams, 0, sizeof(SceGxmInitializeParams));
	initializeParams.flags							= 0;
	initializeParams.displayQueueMaxPendingCount	= DISPLAY_MAX_PENDING_SWAPS;
	initializeParams.displayQueueCallback			= displayCallback;
	initializeParams.displayQueueCallbackDataSize	= sizeof(DisplayData);
	initializeParams.parameterBufferSize			= SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;

	// initialize
	err = sceGxmInitialize(&initializeParams);
	SCE_DBG_ASSERT(err == SCE_OK);

	// allocate ring buffer memory using default sizes
	SceUID vdmRingBufferUid;
	void *vdmRingBuffer = graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE,
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&vdmRingBufferUid);
	SceUID vertexRingBufferUid;
	void *vertexRingBuffer = graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE,
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&vertexRingBufferUid);
	SceUID fragmentRingBufferUid;
	void *fragmentRingBuffer = graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE,
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&fragmentRingBufferUid);
	SceUID fragmentUsseRingBufferUid;
	uint32_t fragmentUsseRingBufferOffset;
	void *fragmentUsseRingBuffer = fragmentUsseAlloc(
		SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE,
		&fragmentUsseRingBufferUid,
		&fragmentUsseRingBufferOffset);

	SceGxmContextParams contextParams;
	memset(&contextParams, 0, sizeof(SceGxmContextParams));
	contextParams.hostMem						= malloc(SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE);
	contextParams.hostMemSize					= SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE;
	contextParams.vdmRingBufferMem				= vdmRingBuffer;
	contextParams.vdmRingBufferMemSize			= SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE;
	contextParams.vertexRingBufferMem			= vertexRingBuffer;
	contextParams.vertexRingBufferMemSize		= SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE;
	contextParams.fragmentRingBufferMem			= fragmentRingBuffer;
	contextParams.fragmentRingBufferMemSize		= SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE;
	contextParams.fragmentUsseRingBufferMem		= fragmentUsseRingBuffer;
	contextParams.fragmentUsseRingBufferMemSize	= SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE;
	contextParams.fragmentUsseRingBufferOffset	= fragmentUsseRingBufferOffset;

	SceGxmContext *context = NULL;
	err = sceGxmCreateContext(&contextParams, &context);
	SCE_DBG_ASSERT(err == SCE_OK);

	// set up parameters
	SceGxmRenderTargetParams renderTargetParams;
	memset(&renderTargetParams, 0, sizeof(SceGxmRenderTargetParams));
	renderTargetParams.flags				= 0;
	renderTargetParams.width				= DISPLAY_WIDTH;
	renderTargetParams.height				= DISPLAY_HEIGHT;
	renderTargetParams.scenesPerFrame		= 1;
	renderTargetParams.multisampleMode		= MSAA_MODE;
	renderTargetParams.multisampleLocations	= 0;
	renderTargetParams.driverMemBlock		= SCE_UID_INVALID_UID;

	/*	If you would like to allocate the memblock manually, then this code can
		be used.  Change the MANUALLY_ALLOCATE_RT_MEMBLOCK to 1 at the top of
		this file to use this mode in the sample.
	*/
#if MANUALLY_ALLOCATE_RT_MEMBLOCK
	{
		// compute memblock size
		uint32_t driverMemSize;
		sceGxmGetRenderTargetMemSize(&renderTargetParams, &driverMemSize);

		// allocate driver memory
		renderTargetParams.driverMemBlock = sceKernelAllocMemBlock(
			"SampleRT",
			SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
			driverMemSize,
			NULL);
	}
#endif

	// create the render target
	SceGxmRenderTarget *renderTarget;
	err = sceGxmCreateRenderTarget(&renderTargetParams, &renderTarget);
	SCE_DBG_ASSERT(err == SCE_OK);

	// allocate memory and sync objects for display buffers
	void *displayBufferData[DISPLAY_BUFFER_COUNT];
	SceUID displayBufferUid[DISPLAY_BUFFER_COUNT];
	SceGxmColorSurface displaySurface[DISPLAY_BUFFER_COUNT];
	SceGxmSyncObject *displayBufferSync[DISPLAY_BUFFER_COUNT];
	for (uint32_t i = 0; i < DISPLAY_BUFFER_COUNT; ++i) {
		// allocate memory for display
		displayBufferData[i] = graphicsAlloc(
			SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA,
			4*DISPLAY_STRIDE_IN_PIXELS*DISPLAY_HEIGHT,
			SCE_GXM_COLOR_SURFACE_ALIGNMENT,
			SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
			&displayBufferUid[i]);

		// memset the buffer to black
		for (uint32_t y = 0; y < DISPLAY_HEIGHT; ++y) {
			uint32_t *row = (uint32_t *)displayBufferData[i] + y*DISPLAY_STRIDE_IN_PIXELS;
			for (uint32_t x = 0; x < DISPLAY_WIDTH; ++x) {
				row[x] = 0xff000000;
			}
		}

		// initialize a color surface for this display buffer
		err = sceGxmColorSurfaceInit(
			&displaySurface[i],
			DISPLAY_COLOR_FORMAT,
			SCE_GXM_COLOR_SURFACE_LINEAR,
			(MSAA_MODE == SCE_GXM_MULTISAMPLE_NONE) ? SCE_GXM_COLOR_SURFACE_SCALE_NONE : SCE_GXM_COLOR_SURFACE_SCALE_MSAA_DOWNSCALE,
			SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT,
			DISPLAY_WIDTH,
			DISPLAY_HEIGHT,
			DISPLAY_STRIDE_IN_PIXELS,
			displayBufferData[i]);
		SCE_DBG_ASSERT(err == SCE_OK);

		// create a sync object that we will associate with this buffer
		err = sceGxmSyncObjectCreate(&displayBufferSync[i]);
		SCE_DBG_ASSERT(err == SCE_OK);
	}

	// compute the memory footprint of the depth buffer
	const uint32_t alignedWidth = ALIGN(DISPLAY_WIDTH, SCE_GXM_TILE_SIZEX);
	const uint32_t alignedHeight = ALIGN(DISPLAY_HEIGHT, SCE_GXM_TILE_SIZEY);
	uint32_t sampleCount = alignedWidth*alignedHeight;
	uint32_t depthStrideInSamples = alignedWidth;
	if (MSAA_MODE == SCE_GXM_MULTISAMPLE_4X) {
		// samples increase in X and Y
		sampleCount *= 4;
		depthStrideInSamples *= 2;
	} else if (MSAA_MODE == SCE_GXM_MULTISAMPLE_2X) {
		// samples increase in Y only
		sampleCount *= 2;
	}

	// allocate it
	SceUID depthBufferUid;
	void *depthBufferData = graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		4*sampleCount,
		SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT,
		SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
		&depthBufferUid);

	// create the SceGxmDepthStencilSurface structure
	SceGxmDepthStencilSurface depthSurface;
	err = sceGxmDepthStencilSurfaceInit(
		&depthSurface,
		SCE_GXM_DEPTH_STENCIL_FORMAT_S8D24,
		SCE_GXM_DEPTH_STENCIL_SURFACE_TILED,
		depthStrideInSamples,
		depthBufferData,
		NULL);
	SCE_DBG_ASSERT(err == SCE_OK);

	// set buffer sizes for this sample
	const uint32_t patcherBufferSize		= 64*1024;
	const uint32_t patcherVertexUsseSize	= 64*1024;
	const uint32_t patcherFragmentUsseSize	= 64*1024;

	// allocate memory for buffers and USSE code
	SceUID patcherBufferUid;
	void *patcherBuffer = graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		patcherBufferSize,
		4,
		SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
		&patcherBufferUid);
	SceUID patcherVertexUsseUid;
	uint32_t patcherVertexUsseOffset;
	void *patcherVertexUsse = vertexUsseAlloc(
		patcherVertexUsseSize,
		&patcherVertexUsseUid,
		&patcherVertexUsseOffset);
	SceUID patcherFragmentUsseUid;
	uint32_t patcherFragmentUsseOffset;
	void *patcherFragmentUsse = fragmentUsseAlloc(
		patcherFragmentUsseSize,
		&patcherFragmentUsseUid,
		&patcherFragmentUsseOffset);

	// create a shader patcher
	SceGxmShaderPatcherParams patcherParams;
	memset(&patcherParams, 0, sizeof(SceGxmShaderPatcherParams));
	patcherParams.userData					= NULL;
	patcherParams.hostAllocCallback			= &patcherHostAlloc;
	patcherParams.hostFreeCallback			= &patcherHostFree;
	patcherParams.bufferAllocCallback		= NULL;
	patcherParams.bufferFreeCallback		= NULL;
	patcherParams.bufferMem					= patcherBuffer;
	patcherParams.bufferMemSize				= patcherBufferSize;
	patcherParams.vertexUsseAllocCallback	= NULL;
	patcherParams.vertexUsseFreeCallback	= NULL;
	patcherParams.vertexUsseMem				= patcherVertexUsse;
	patcherParams.vertexUsseMemSize			= patcherVertexUsseSize;
	patcherParams.vertexUsseOffset			= patcherVertexUsseOffset;
	patcherParams.fragmentUsseAllocCallback	= NULL;
	patcherParams.fragmentUsseFreeCallback	= NULL;
	patcherParams.fragmentUsseMem			= patcherFragmentUsse;
	patcherParams.fragmentUsseMemSize		= patcherFragmentUsseSize;
	patcherParams.fragmentUsseOffset		= patcherFragmentUsseOffset;

	SceGxmShaderPatcher *shaderPatcher = NULL;
	err = sceGxmShaderPatcherCreate(&patcherParams, &shaderPatcher);
	SCE_DBG_ASSERT(err == SCE_OK);

	// load and start SceShaccCg
	SceUID shaccCgId = sceKernelLoadStartModule("app0:/module/libshacccg.suprx", 0, NULL, 0, NULL, NULL);
	SCE_DBG_ASSERT(shaccCgId > 0);

	// set memory allocator for SceShaccCg
	err = sceShaccCgSetMemAllocator(malloc, free);
	SCE_DBG_ASSERT(err == SCE_OK);

	// create shader compiler options
	SceShaccCgCompileOptions compileOpts;
	sceShaccCgInitializeCompileOptions(&compileOpts);

	// create shader compiler callback list
	SceShaccCgCallbackList callbackList;
	sceShaccCgInitializeCallbackList(&callbackList, SCE_SHACCCG_TRIVIAL);
	callbackList.openFile = openFileCallback;
	callbackList.releaseFile = releaseFileCallback;

	// compile shaders
	const SceShaccCgCompileOutput *clearVertexCompileOutput;
	const SceShaccCgCompileOutput *clearFragmentCompileOutput;
	const SceShaccCgCompileOutput *basicVertexCompileOutput;
	const SceShaccCgCompileOutput *basicFragmentCompileOutput;

	compileOpts.targetProfile = SCE_SHACCCG_PROFILE_VP;
	compileOpts.mainSourceFile = "app0:/shader/clear_v.cg";
	clearVertexCompileOutput = sceShaccCgCompileProgram(&compileOpts, &callbackList, NULL);
	SCE_DBG_ASSERT(clearVertexCompileOutput != NULL && clearVertexCompileOutput->programData != NULL);

	compileOpts.targetProfile = SCE_SHACCCG_PROFILE_FP;
	compileOpts.mainSourceFile = "app0:/shader/clear_f.cg";
	clearFragmentCompileOutput = sceShaccCgCompileProgram(&compileOpts, &callbackList, NULL);
	SCE_DBG_ASSERT(clearFragmentCompileOutput != NULL && clearFragmentCompileOutput->programData != NULL);

	compileOpts.targetProfile = SCE_SHACCCG_PROFILE_VP;
	compileOpts.mainSourceFile = "app0:/shader/basic_v.cg";
	basicVertexCompileOutput = sceShaccCgCompileProgram(&compileOpts, &callbackList, NULL);
	SCE_DBG_ASSERT(basicVertexCompileOutput != NULL && basicVertexCompileOutput->programData != NULL);

	compileOpts.targetProfile = SCE_SHACCCG_PROFILE_FP;
	compileOpts.mainSourceFile = "app0:/shader/basic_f.cg";
	basicFragmentCompileOutput = sceShaccCgCompileProgram(&compileOpts, &callbackList, NULL);
	SCE_DBG_ASSERT(basicFragmentCompileOutput != NULL && basicFragmentCompileOutput->programData != NULL);

	const SceGxmProgram *clearVertexProgramGxp = (const SceGxmProgram *)clearVertexCompileOutput->programData;
	const SceGxmProgram *clearFragmentProgramGxp = (const SceGxmProgram *)clearFragmentCompileOutput->programData;
	const SceGxmProgram *basicVertexProgramGxp = (const SceGxmProgram *)basicVertexCompileOutput->programData;
	const SceGxmProgram *basicFragmentProgramGxp = (const SceGxmProgram *)basicFragmentCompileOutput->programData;

	// register programs with the patcher
	SceGxmShaderPatcherId clearVertexProgramId;
	SceGxmShaderPatcherId clearFragmentProgramId;
	SceGxmShaderPatcherId basicVertexProgramId;
	SceGxmShaderPatcherId basicFragmentProgramId;
	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, clearVertexProgramGxp, &clearVertexProgramId);
	SCE_DBG_ASSERT(err == SCE_OK);
	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, clearFragmentProgramGxp, &clearFragmentProgramId);
	SCE_DBG_ASSERT(err == SCE_OK);
	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, basicVertexProgramGxp, &basicVertexProgramId);
	SCE_DBG_ASSERT(err == SCE_OK);
	err = sceGxmShaderPatcherRegisterProgram(shaderPatcher, basicFragmentProgramGxp, &basicFragmentProgramId);
	SCE_DBG_ASSERT(err == SCE_OK);

	// get attributes by name to create vertex format bindings
	const SceGxmProgramParameter *paramClearPositionAttribute = sceGxmProgramFindParameterByName(clearVertexProgramGxp, "aPosition");
	SCE_DBG_ASSERT(paramClearPositionAttribute && (sceGxmProgramParameterGetCategory(paramClearPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE));

	// create clear vertex format
	SceGxmVertexAttribute clearVertexAttributes[1];
	SceGxmVertexStream clearVertexStreams[1];
	clearVertexAttributes[0].streamIndex = 0;
	clearVertexAttributes[0].offset = 0;
	clearVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	clearVertexAttributes[0].componentCount = 2;
	clearVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramClearPositionAttribute);
	clearVertexStreams[0].stride = sizeof(ClearVertex);
	clearVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	// create clear programs
	SceGxmVertexProgram *clearVertexProgram = NULL;
	SceGxmFragmentProgram *clearFragmentProgram = NULL;
	err = sceGxmShaderPatcherCreateVertexProgram(
		shaderPatcher,
		clearVertexProgramId,
		clearVertexAttributes,
		1,
		clearVertexStreams,
		1,
		&clearVertexProgram);
	SCE_DBG_ASSERT(err == SCE_OK);
	err = sceGxmShaderPatcherCreateFragmentProgram(
		shaderPatcher,
		clearFragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		MSAA_MODE,
		NULL,
		clearVertexProgramGxp,
		&clearFragmentProgram);
	SCE_DBG_ASSERT(err == SCE_OK);

	// create the clear triangle vertex/index data
	SceUID clearVerticesUid;
	SceUID clearIndicesUid;
	ClearVertex *const clearVertices = (ClearVertex *)graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		3*sizeof(ClearVertex),
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&clearVerticesUid);
	uint16_t *const clearIndices = (uint16_t *)graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		3*sizeof(uint16_t),
		2,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&clearIndicesUid);

	clearVertices[0].x = -1.0f;
	clearVertices[0].y = -1.0f;
	clearVertices[1].x =  3.0f;
	clearVertices[1].y = -1.0f;
	clearVertices[2].x = -1.0f;
	clearVertices[2].y =  3.0f;

	clearIndices[0] = 0;
	clearIndices[1] = 1;
	clearIndices[2] = 2;

	// get attributes by name to create vertex format bindings
	// first retrieve the underlying program to extract binding information
	const SceGxmProgramParameter *paramBasicPositionAttribute = sceGxmProgramFindParameterByName(basicVertexProgramGxp, "aPosition");
	SCE_DBG_ASSERT(paramBasicPositionAttribute && (sceGxmProgramParameterGetCategory(paramBasicPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE));
	const SceGxmProgramParameter *paramBasicColorAttribute = sceGxmProgramFindParameterByName(basicVertexProgramGxp, "aColor");
	SCE_DBG_ASSERT(paramBasicColorAttribute && (sceGxmProgramParameterGetCategory(paramBasicColorAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE));

	// create shaded triangle vertex format
	SceGxmVertexAttribute basicVertexAttributes[2];
	SceGxmVertexStream basicVertexStreams[1];
	basicVertexAttributes[0].streamIndex = 0;
	basicVertexAttributes[0].offset = 0;
	basicVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	basicVertexAttributes[0].componentCount = 3;
	basicVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramBasicPositionAttribute);
	basicVertexAttributes[1].streamIndex = 0;
	basicVertexAttributes[1].offset = 12;
	basicVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N;
	basicVertexAttributes[1].componentCount = 4;
	basicVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramBasicColorAttribute);
	basicVertexStreams[0].stride = sizeof(BasicVertex);
	basicVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	// create shaded triangle shaders
	SceGxmVertexProgram *basicVertexProgram = NULL;
	SceGxmFragmentProgram *basicFragmentProgram = NULL;
	err = sceGxmShaderPatcherCreateVertexProgram(
		shaderPatcher,
		basicVertexProgramId,
		basicVertexAttributes,
		2,
		basicVertexStreams,
		1,
		&basicVertexProgram);
	SCE_DBG_ASSERT(err == SCE_OK);
	err = sceGxmShaderPatcherCreateFragmentProgram(
		shaderPatcher,
		basicFragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		MSAA_MODE,
		NULL,
		basicVertexProgramGxp,
		&basicFragmentProgram);
	SCE_DBG_ASSERT(err == SCE_OK);

	// find vertex uniforms by name and cache parameter information
	const SceGxmProgramParameter *wvpParam = sceGxmProgramFindParameterByName(basicVertexProgramGxp, "wvp");
	SCE_DBG_ASSERT(wvpParam && (sceGxmProgramParameterGetCategory(wvpParam) == SCE_GXM_PARAMETER_CATEGORY_UNIFORM));

	// create shaded triangle vertex/index data
	SceUID basicVerticesUid;
	SceUID basicIndiceUid;
	BasicVertex *const basicVertices = (BasicVertex *)graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		3*sizeof(BasicVertex),
		4,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&basicVerticesUid);
	uint16_t *const basicIndices = (uint16_t *)graphicsAlloc(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
		3*sizeof(uint16_t),
		2,
		SCE_GXM_MEMORY_ATTRIB_READ,
		&basicIndiceUid);

	basicVertices[0].x = -0.5f;
	basicVertices[0].y = -0.5f;
	basicVertices[0].z = 0.0f;
	basicVertices[0].color = 0xff0000ff;
	basicVertices[1].x = 0.5f;
	basicVertices[1].y = -0.5f;
	basicVertices[1].z = 0.0f;
	basicVertices[1].color = 0xff00ff00;
	basicVertices[2].x = -0.5f;
	basicVertices[2].y = 0.5f;
	basicVertices[2].z = 0.0f;
	basicVertices[2].color = 0xffff0000;

	basicIndices[0] = 0;
	basicIndices[1] = 1;
	basicIndices[2] = 2;

	// initialize controller data
	SceCtrlData ctrlData;
	memset(&ctrlData, 0, sizeof(ctrlData));

	// message for SDK sample auto test
	printf("## api_libgxm/basic: INIT SUCCEEDED ##\n");

	// loop until exit
	uint32_t backBufferIndex = 0;
	uint32_t frontBufferIndex = 0;
	float rotationAngle = 0.0f;
	bool quit = false;
	while (!quit) {

		// check control data
		sceCtrlPeekBufferPositive(0, &ctrlData, 1);

		// update triangle angle
		rotationAngle += SCE_MATH_TWOPI/60.0f;
		if (rotationAngle > SCE_MATH_TWOPI)
			rotationAngle -= SCE_MATH_TWOPI;

		// set up a 4x4 matrix for a rotation
		float aspectRatio = (float)DISPLAY_WIDTH/(float)DISPLAY_HEIGHT;

		float s = sin(rotationAngle);
		float c = cos(rotationAngle);

		float wvpData[16];
		wvpData[ 0] = c/aspectRatio;
		wvpData[ 1] = s;
		wvpData[ 2] = 0.0f;
		wvpData[ 3] = 0.0f;

		wvpData[ 4] = -s/aspectRatio;
		wvpData[ 5] = c;
		wvpData[ 6] = 0.0f;
		wvpData[ 7] = 0.0f;

		wvpData[ 8] = 0.0f;
		wvpData[ 9] = 0.0f;
		wvpData[10] = 1.0f;
		wvpData[11] = 0.0f;

		wvpData[12] = 0.0f;
		wvpData[13] = 0.0f;
		wvpData[14] = 0.0f;
		wvpData[15] = 1.0f;

		// start rendering to the main render target
		sceGxmBeginScene(
			context,
			0,
			renderTarget,
			NULL,
			NULL,
			displayBufferSync[backBufferIndex],
			&displaySurface[backBufferIndex],
			&depthSurface);

		// set clear shaders
		sceGxmSetVertexProgram(context, clearVertexProgram);
		sceGxmSetFragmentProgram(context, clearFragmentProgram);

		// draw the clear triangle
		sceGxmSetVertexStream(context, 0, clearVertices);
		sceGxmDraw(context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, clearIndices, 3);

		// render the rotating triangle
		sceGxmSetVertexProgram(context, basicVertexProgram);
		sceGxmSetFragmentProgram(context, basicFragmentProgram);

		// set the vertex program constants
		void *vertexDefaultBuffer;
		sceGxmReserveVertexDefaultUniformBuffer(context, &vertexDefaultBuffer);
		sceGxmSetUniformDataF(vertexDefaultBuffer, wvpParam, 0, 16, wvpData);

		// draw the spinning triangle
		sceGxmSetVertexStream(context, 0, basicVertices);
		sceGxmDraw(context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, basicIndices, 3);

		// end the scene on the main render target, submitting rendering work to the GPU
		sceGxmEndScene(context, NULL, NULL);

		// PA heartbeat to notify end of frame
		sceGxmPadHeartbeat(&displaySurface[backBufferIndex], displayBufferSync[backBufferIndex]);

		// queue the display swap for this frame
		DisplayData displayData;
		displayData.address = displayBufferData[backBufferIndex];
		sceGxmDisplayQueueAddEntry(
			displayBufferSync[frontBufferIndex],	// front buffer is OLD buffer
			displayBufferSync[backBufferIndex],		// back buffer is NEW buffer
			&displayData);

		// update buffer indices
		frontBufferIndex = backBufferIndex;
		backBufferIndex = (backBufferIndex + 1) % DISPLAY_BUFFER_COUNT;
	}

	// wait until rendering is done
	sceGxmFinish(context);

	// clean up allocations
	sceGxmShaderPatcherReleaseFragmentProgram(shaderPatcher, basicFragmentProgram);
	sceGxmShaderPatcherReleaseVertexProgram(shaderPatcher, basicVertexProgram);
	sceGxmShaderPatcherReleaseFragmentProgram(shaderPatcher, clearFragmentProgram);
	sceGxmShaderPatcherReleaseVertexProgram(shaderPatcher, clearVertexProgram);
	graphicsFree(basicIndiceUid);
	graphicsFree(basicVerticesUid);
	graphicsFree(clearIndicesUid);
	graphicsFree(clearVerticesUid);

	// wait until display queue is finished before deallocating display buffers
	err = sceGxmDisplayQueueFinish();
	SCE_DBG_ASSERT(err == SCE_OK);

	// clean up display queue
	graphicsFree(depthBufferUid);
	for (uint32_t i = 0; i < DISPLAY_BUFFER_COUNT; ++i) {
		// clear the buffer then deallocate
		memset(displayBufferData[i], 0, DISPLAY_HEIGHT*DISPLAY_STRIDE_IN_PIXELS*4);
		graphicsFree(displayBufferUid[i]);

		// destroy the sync object
		sceGxmSyncObjectDestroy(displayBufferSync[i]);
	}

	// unregister programs
	sceGxmShaderPatcherUnregisterProgram(shaderPatcher, basicFragmentProgramId);
	sceGxmShaderPatcherUnregisterProgram(shaderPatcher, basicVertexProgramId);
	sceGxmShaderPatcherUnregisterProgram(shaderPatcher, clearFragmentProgramId);
	sceGxmShaderPatcherUnregisterProgram(shaderPatcher, clearVertexProgramId);

	// destroy compiled shaders
	sceShaccCgDestroyCompileOutput(basicFragmentCompileOutput);
	sceShaccCgDestroyCompileOutput(basicVertexCompileOutput);
	sceShaccCgDestroyCompileOutput(clearFragmentCompileOutput);
	sceShaccCgDestroyCompileOutput(clearVertexCompileOutput);

	// stop and unload SceShaccCg
	sceKernelStopUnloadModule(shaccCgId, 0, NULL, 0, NULL, NULL);

	// destroy shader patcher
	sceGxmShaderPatcherDestroy(shaderPatcher);
	fragmentUsseFree(patcherFragmentUsseUid);
	vertexUsseFree(patcherVertexUsseUid);
	graphicsFree(patcherBufferUid);

	// destroy the render target
	sceGxmDestroyRenderTarget(renderTarget);
#if MANUALLY_ALLOCATE_RT_MEMBLOCK
	sceKernelFreeMemBlock(renderTargetParams.driverMemBlock);
#endif

	// destroy the context
	sceGxmDestroyContext(context);
	fragmentUsseFree(fragmentUsseRingBufferUid);
	graphicsFree(fragmentRingBufferUid);
	graphicsFree(vertexRingBufferUid);
	graphicsFree(vdmRingBufferUid);
	free(contextParams.hostMem);

	// terminate libgxm
	sceGxmTerminate();

	// message for SDK sample auto test
	printf("## api_libgxm/basic: FINISHED ##\n");
	return SCE_OK;
}

void displayCallback(const void *callbackData)
{
	SceDisplayFrameBuf framebuf;
	int err = SCE_OK;
	UNUSED(err);

	// Cast the parameters back
	const DisplayData *displayData = (const DisplayData *)callbackData;

	// Swap to the new buffer on the next VSYNC
	memset(&framebuf, 0x00, sizeof(SceDisplayFrameBuf));
	framebuf.size        = sizeof(SceDisplayFrameBuf);
	framebuf.base        = displayData->address;
	framebuf.pitch       = DISPLAY_STRIDE_IN_PIXELS;
	framebuf.pixelformat = DISPLAY_PIXEL_FORMAT;
	framebuf.width       = DISPLAY_WIDTH;
	framebuf.height      = DISPLAY_HEIGHT;
	err = sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_UPDATETIMING_NEXTVSYNC);
	SCE_DBG_ASSERT(err == SCE_OK);

	// Block this callback until the swap has occurred and the old buffer
	// is no longer displayed
	err = sceDisplayWaitVblankStart();
	SCE_DBG_ASSERT(err == SCE_OK);
}

static void *patcherHostAlloc(void *userData, uint32_t size)
{
	UNUSED(userData);
	return malloc(size);
}

static void patcherHostFree(void *userData, void *mem)
{
	UNUSED(userData);
	free(mem);
}

static void *graphicsAlloc(SceKernelMemBlockType type, uint32_t size, uint32_t alignment, uint32_t attribs, SceUID *uid)
{
	int err = SCE_OK;
	UNUSED(err);

	/*	Since we are using sceKernelAllocMemBlock directly, we cannot directly
		use the alignment parameter.  Instead, we must allocate the size to the
		minimum for this memblock type, and just assert that this will cover
		our desired alignment.

		Developers using their own heaps should be able to use the alignment
		parameter directly for more minimal padding.
	*/
	if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA) {
		// CDRAM memblocks must be 256KiB aligned
		SCE_DBG_ASSERT(alignment <= 256*1024);
		size = ALIGN(size, 256*1024);
	} else {
		// LPDDR memblocks must be 4KiB aligned
		SCE_DBG_ASSERT(alignment <= 4*1024);
		size = ALIGN(size, 4*1024);
	}
	UNUSED(alignment);

	// allocate some memory
	*uid = sceKernelAllocMemBlock("basic", type, size, NULL);
	SCE_DBG_ASSERT(*uid >= SCE_OK);

	// grab the base address
	void *mem = NULL;
	err = sceKernelGetMemBlockBase(*uid, &mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// map for the GPU
	err = sceGxmMapMemory(mem, size, attribs);
	SCE_DBG_ASSERT(err == SCE_OK);

	// done
	return mem;
}

static void graphicsFree(SceUID uid)
{
	int err = SCE_OK;
	UNUSED(err);

	// grab the base address
	void *mem = NULL;
	err = sceKernelGetMemBlockBase(uid, &mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// unmap memory
	err = sceGxmUnmapMemory(mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// free the memory block
	err = sceKernelFreeMemBlock(uid);
	SCE_DBG_ASSERT(err == SCE_OK);
}

static void *vertexUsseAlloc(uint32_t size, SceUID *uid, uint32_t *usseOffset)
{
	int err = SCE_OK;
	UNUSED(err);

	// align to memblock alignment for LPDDR
	size = ALIGN(size, 4096);

	// allocate some memory
	*uid = sceKernelAllocMemBlock("basic", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, size, NULL);
	SCE_DBG_ASSERT(*uid >= SCE_OK);

	// grab the base address
	void *mem = NULL;
	err = sceKernelGetMemBlockBase(*uid, &mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// map as vertex USSE code for the GPU
	err = sceGxmMapVertexUsseMemory(mem, size, usseOffset);
	SCE_DBG_ASSERT(err == SCE_OK);

	// done
	return mem;
}

static void vertexUsseFree(SceUID uid)
{
	int err = SCE_OK;
	UNUSED(err);

	// grab the base address
	void *mem = NULL;
	err = sceKernelGetMemBlockBase(uid, &mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// unmap memory
	err = sceGxmUnmapVertexUsseMemory(mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// free the memory block
	err = sceKernelFreeMemBlock(uid);
	SCE_DBG_ASSERT(err == SCE_OK);
}

static void *fragmentUsseAlloc(uint32_t size, SceUID *uid, uint32_t *usseOffset)
{
	int err = SCE_OK;
	UNUSED(err);

	// align to memblock alignment for LPDDR
	size = ALIGN(size, 4096);

	// allocate some memory
	*uid = sceKernelAllocMemBlock("basic", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, size, NULL);
	SCE_DBG_ASSERT(*uid >= SCE_OK);

	// grab the base address
	void *mem = NULL;
	err = sceKernelGetMemBlockBase(*uid, &mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// map as fragment USSE code for the GPU
	err = sceGxmMapFragmentUsseMemory(mem, size, usseOffset);
	SCE_DBG_ASSERT(err == SCE_OK);

	// done
	return mem;
}

static void fragmentUsseFree(SceUID uid)
{
	int err = SCE_OK;
	UNUSED(err);

	// grab the base address
	void *mem = NULL;
	err = sceKernelGetMemBlockBase(uid, &mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// unmap memory
	err = sceGxmUnmapFragmentUsseMemory(mem);
	SCE_DBG_ASSERT(err == SCE_OK);

	// free the memory block
	err = sceKernelFreeMemBlock(uid);
	SCE_DBG_ASSERT(err == SCE_OK);
}

static SceShaccCgSourceFile *openFileCallback(
	const char *fileName,
	const SceShaccCgSourceLocation *includedFrom,
	const SceShaccCgCompileOptions *compileOptions,
	void *userData,
	const char **errorString)
{
	UNUSED(includedFrom);
	UNUSED(compileOptions);
	UNUSED(userData);
	UNUSED(errorString);

	int err = SCE_OK;
	UNUSED(err);

	// open file
	SceUID uid = sceIoOpen(fileName, SCE_O_RDONLY, 0);
	SCE_DBG_ASSERT(uid >= 0);

	// get file size
	SceIoStat stat;
	memset(&stat, 0, sizeof(stat));
	err = sceIoGetstatByFd(uid, &stat);
	SCE_DBG_ASSERT(err == SCE_OK);

	// allocate memory for file contents
	void *buf = malloc((SceUInt32)stat.st_size);
	SCE_DBG_ASSERT(buf != NULL);

	// read file
	err = sceIoRead(uid, buf, (SceUInt32)stat.st_size);
	SCE_DBG_ASSERT(err == stat.st_size);

	// close file
	err = sceIoClose(uid);
	SCE_DBG_ASSERT(err == SCE_OK);

	// allocate and set return value
	SceShaccCgSourceFile *sourceFile = malloc(sizeof(SceShaccCgSourceFile));
	SCE_DBG_ASSERT(sourceFile != NULL);
	sourceFile->fileName = fileName;
	sourceFile->text = buf;
	sourceFile->size = stat.st_size;

	// done
	return sourceFile;
}

static void releaseFileCallback(
	const SceShaccCgSourceFile *file,
	const SceShaccCgCompileOptions *compileOptions,
	void *userData)
{
	UNUSED(compileOptions);
	UNUSED(userData);

	// free memory for file contents
	free((void *)file->text);

	// free memory for file struct
	free((void *)file);
}
