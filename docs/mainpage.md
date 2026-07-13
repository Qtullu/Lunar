@mainpage Lunar Plugin



<b>Unreal Engine 5.8 development plugin</b><br>

by <b>Edgar Frolenkov</b><br>

<a href="https://qtullu.github.io/Lunar/">Online Documentation</a>



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_overview Overview



<b>Lunar Plugin</b> is a development acceleration plugin for Unreal Engine 5.8



It provides a growing set of runtime subsystems Blueprint function libraries shared types widgets settings and utility tools designed to reduce repetitive project code and make common game development tasks faster to implement



The plugin is intended as a reusable foundation for Unreal Engine projects and includes systems and helpers for console tooling performance monitoring raw input tracking file operations platform queries transform manipulation save helpers UI utilities audio utilities debugging math randomization data tables JSON physics blockout workflows materials actors sound design and other production utilities



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_sections Documentation sections



<ul>

<li>@ref LunarFunctionLibraries</li>

<li>@ref LunarSubsystems</li>

<li>@ref LunarComponents</li>

<li>@ref LunarSettings</li>

<li>@ref LunarTypes</li>

<li>@ref LunarWidgets</li>

</ul>



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_subsystems Subsystems



Subsystems are persistent runtime services used by the plugin and exposed to Blueprint where needed



@subsection lunar\_console\_subsystem Console



@ref LunarConsoleSubsystem stores console messages executes Lunar console commands manages command suggestions controls the console widget and broadcasts console events



@subsection lunar\_performance\_subsystem Performance



@ref LunarPerformanceSubsystem collects runtime performance snapshots stores performance history tracks FPS frame time memory GPU CPU disk metrics and controls the performance widget



@subsection lunar\_raw\_input\_subsystem Raw Input



@ref LunarRawInputSubsystem tracks raw input state detects the last active input device stores key mouse and wheel input snapshots and broadcasts raw input events



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_components Components



@ref LunarAutoRotatorComponent rotates an actor or scene component to face Player Controller 0 camera an editor viewport camera or an overridden actor or scene component target with axis filtering rotation offset relative rotation and optional interpolation



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_function\_libraries Function libraries



Function libraries expose small focused Blueprint utility functions



@subsection lunar\_fl\_game Game



@ref LunarFLGame provides build configuration runtime environment platform project and clipboard helpers



@subsection lunar\_fl\_networking Networking



@ref LunarFLNetworking provides networking helper functions



@subsection lunar\_fl\_math Math



@ref LunarFLMath provides math helper functions



@subsection lunar\_fl\_random Random



@ref LunarFLRandom provides random value helper functions



@subsection lunar\_fl\_string String



@ref LunarFLString provides string formatting helper functions



@subsection lunar\_fl\_chrono Chrono



@ref LunarFLChrono provides time and chrono helper functions



@subsection lunar\_fl\_ui UI



@ref LunarFLUI provides user interface helper functions



@subsection lunar\_fl\_debug Debug



@ref LunarFLDebug provides debug helper functions



@subsection lunar\_fl\_audio Audio



@ref LunarFLAudio provides audio helper functions



@subsection lunar\_fl\_data\_table Data Table



@ref LunarFLDataTable provides data table helper functions



@subsection lunar\_fl\_save Save



@ref LunarFLSave provides save helper functions



@subsection lunar\_fl\_physics Physics



@ref LunarFLPhysics provides physics helper functions



@subsection lunar\_fl\_blockout Blockout



@ref LunarFLBlockout provides blockout workflow helper functions



@subsection lunar\_fl\_json JSON



@ref LunarFLJSON provides JSON helper functions



@subsection lunar\_fl\_performance Performance



@ref LunarFLPerformance provides runtime performance metric snapshot summary history and graph painting helpers



@subsection lunar\_fl\_console Console



@ref LunarFLConsole provides Lunar console print command execution command suggestion and log export helpers



@subsection lunar\_fl\_transform Transform



@ref LunarFLTransform provides actor and scene component transform axis helpers



@subsection lunar\_fl\_file File



@ref LunarFLFile provides native folder selection and text file saving helpers



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_types Types



Types contain shared enums structs settings records snapshots and data definitions used by the plugin



@subsection lunar\_types\_raw\_input Raw Input



@ref LunarTypesRawInput contains raw input device and input snapshot types



@subsection lunar\_types\_aggregate Aggregate



@ref LunarTypesAggregate contains the aggregate header for shared Lunar types



@subsection lunar\_types\_audio Audio



@ref LunarTypesAudio contains audio shared types



@subsection lunar\_types\_blockout Blockout



@ref LunarTypesBlockout contains blockout shared types



@subsection lunar\_types\_chrono Chrono



@ref LunarTypesChrono contains time shared types



@subsection lunar\_types\_console Console



@ref LunarTypesConsole contains console message command table command definition command parameter and console settings types



@subsection lunar\_types\_data\_table Data Table



@ref LunarTypesDataTable contains data table shared types



@subsection lunar\_types\_debug Debug



@ref LunarTypesDebug contains debug shared types



@subsection lunar\_types\_game Game



@ref LunarTypesGame contains platform type and platform family types



@subsection lunar\_types\_json JSON



@ref LunarTypesJSON contains JSON shared types



@subsection lunar\_types\_math Math



@ref LunarTypesMath contains math shared types



@subsection lunar\_types\_networking Networking



@ref LunarTypesNetworking contains networking shared types



@subsection lunar\_types\_performance Performance



@ref LunarTypesPerformance contains performance memory units snapshots histories metric structures and performance settings



@subsection lunar\_types\_physics Physics



@ref LunarTypesPhysics contains physics shared types



@subsection lunar\_types\_random Random



@ref LunarTypesRandom contains random shared types



@subsection lunar\_types\_save Save



@ref LunarTypesSave contains save shared types



@subsection lunar\_types\_string String



@ref LunarTypesString contains string shared types



@subsection lunar\_types\_transform Transform



@ref LunarTypesTransform contains transform axis and transform data type enums



@subsection lunar\_types\_ui UI



@ref LunarTypesUI contains user interface shared types



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_settings Settings



@ref LunarPluginSettings stores project level Lunar settings exposed through Unreal Developer Settings



The settings currently configure Lunar console defaults and Lunar performance monitoring defaults



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_widgets Widgets



@ref LunarDraggableWindow is a reusable draggable and resizable UMG window widget with bounds resize modes handle states and movement events



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_public\_headers Public headers



The plugin exposes aggregate headers for faster access to public API groups



@subsection lunar\_header Lunar.h



Main public plugin include



@subsection lunar\_fl\_header LunarFL.h



Aggregate include for all Lunar function libraries



@htmlonly

<div class="fragment lunar-code-example">

<div class="line"><span class="preprocessor">#include</span> <span class="stringliteral">"LunarFL.h"</span></div>

<div class="line"></div>

<div class="line">FString PlatformName = LunarFL::Game::GetPlatformName();</div>

<div class="line">FString PercentText = LunarFL::String::FormatPercent01(0.75f);</div>

</div>

@endhtmlonly



@subsection lunar\_types\_header LunarTypes.h



Aggregate include for shared Lunar type headers



@htmlonly

<div class="fragment lunar-code-example">

<div class="line"><span class="preprocessor">#include</span> <span class="stringliteral">"LunarTypes.h"</span></div>

<div class="line"></div>

<div class="line">ELunarAxisFull Axis = ELunarAxisFull::XYZ;</div>

<div class="line">FLunarRawInputSnapshot InputSnapshot;</div>

<div class="line">ELunarInputDeviceType InputDevice = InputSnapshot.LastInputDevice;</div>

</div>

@endhtmlonly



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_cpp\_example C++ usage example



@htmlonly

<div class="fragment lunar-code-example">

<div class="line"><span class="preprocessor">#include</span> <span class="stringliteral">"LunarFL.h"</span></div>

<div class="line"></div>

<div class="line">FString ProjectName = LunarFL::Game::GetProjectName();</div>

<div class="line"></div>

<div class="line"><span class="keywordtype">bool</span> bSaved = LunarFL::File::SaveTextFileAs(</div>

<div class="line">    WorldContextObject,</div>

<div class="line">    DirectoryPath,</div>

<div class="line">    FileNameWithExtension,</div>

<div class="line">    Text,</div>

<div class="line">    OutSavedFilePath,</div>

<div class="line">    OutError</div>

<div class="line">);</div>

</div>

@endhtmlonly



@htmlonly

<hr class="lunar-section-divider">

@endhtmlonly



@section lunar\_target Development target



<ul>

<li>Engine: Unreal Engine 5.8</li>

<li>Plugin: Lunar</li>

<li>Author: Edgar Frolenkov</li>

<li>Documentation: Doxygen Graphviz Doxygen Awesome CSS</li>

</ul>

