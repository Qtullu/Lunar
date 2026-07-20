// Copyright 2026 Edgar Frolenkov All rights reserved.

/**
 * @file LunarAssetCreationMenu.cpp
 * @brief Implements the editor-only Lunar Content Browser creation menu.
 * @ingroup LunarNavigationEditor
 */

#include "UI/Navigation/Creation/LunarAssetCreationMenu.h"

#include "AssetToolsModule.h"
#include "ContentBrowserDataMenuContexts.h"
#include "ContentBrowserDataSubsystem.h"
#include "ContentBrowserModule.h"
#include "Engine/Texture2D.h"
#include "Factories/BlueprintFactory.h"
#include "Factories/DataAssetFactory.h"
#include "IAssetTools.h"
#include "IContentBrowserDataModule.h"
#include "IContentBrowserSingleton.h"
#include "Misc/PackageName.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateIconFinder.h"
#include "Styling/StyleColors.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"
#include "ToolMenus.h"
#include "UI/Navigation/Controls/LunarButton.h"
#include "UI/Navigation/Controls/LunarComboBox.h"
#include "UI/Navigation/Controls/LunarComboBoxEmptyVisual.h"
#include "UI/Navigation/Controls/LunarComboBoxEntry.h"
#include "UI/Navigation/Controls/LunarComboBoxSelectedVisual.h"
#include "UI/Navigation/Controls/LunarContextMenu.h"
#include "UI/Navigation/Controls/LunarListView.h"
#include "UI/Navigation/Controls/LunarListViewEntry.h"
#include "UI/Navigation/Controls/LunarOptionSlider.h"
#include "UI/Navigation/Controls/LunarRadio.h"
#include "UI/Navigation/Controls/LunarRadioSideVisual.h"
#include "UI/Navigation/Controls/LunarScrollBox.h"
#include "UI/Navigation/Controls/LunarSlider.h"
#include "UI/Navigation/Controls/LunarSwitch.h"
#include "UI/Navigation/Controls/LunarTabHeader.h"
#include "UI/Navigation/Controls/LunarTabs.h"
#include "UI/Navigation/Core/LunarNavigableWidget.h"
#include "UI/Navigation/Core/LunarScreenWidget.h"
#include "UI/Navigation/Data/LunarInputIconSet.h"
#include "UI/Navigation/Data/LunarUIActionRegistry.h"
#include "UI/Navigation/Data/LunarUIHapticFeedbackAsset.h"
#include "UI/Navigation/Data/LunarUISoundFeedbackAsset.h"
#include "UI/Navigation/Prompts/LunarInputPromptWidget.h"
#include "UObject/GCObjectScopeGuard.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "WidgetBlueprintFactory.h"

#define LOCTEXT_NAMESPACE "LunarAssetCreationMenu"

namespace
{
	/** @brief Stable section name used for Lunar asset commands inside their submenus. */
	const FName LunarAssetSectionName(TEXT("LunarAssets"));

	/** @brief Dark purple accent used by the Lunar root asset tile. */
	const FLinearColor LunarRootMenuAccent = FLinearColor::FromSRGBColor(FColor(0x5B, 0x4B, 0x8A));

	/** @brief Matches Epic's compact factory-entry layout while allowing a Lunar-owned icon and color. */
	class SLunarAssetMenuEntry final : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(SLunarAssetMenuEntry)
			: _Icon(nullptr)
			, _AccentColor(FLinearColor::White)
		{}
			SLATE_ARGUMENT(const FSlateBrush*, Icon)
			SLATE_ARGUMENT(FText, Label)
			SLATE_ARGUMENT(FLinearColor, AccentColor)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			ChildSlot
			.Padding(FMargin(0.0f, -1.0f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.0f, 0.0f, 3.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(24.0f)
					.HeightOverride(24.0f)
					[
						SNew(SOverlay)

						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FSlateColor(FStyleColors::Background))
							.Padding(2.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.DesiredSizeOverride(FVector2D(16.0f, 16.0f))
								.ColorAndOpacity(FSlateColor::UseForeground())
								.Image(InArgs._Icon)
							]
						]

						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Bottom)
						[
							SNew(SColorBlock)
							.Size(FVector2D(1.0f, 2.0f))
							.Color(InArgs._AccentColor)
						]
					]
				]

				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(STextBlock)
					.Font(FAppStyle::GetFontStyle("LevelViewportContextMenu.AssetLabel.Text.Font"))
					.Text(InArgs._Label)
				]
			];
		}
	};

	/** @brief Applies the Content Browser Add New visual style to a generated submenu. */
	void ApplyContentBrowserMenuStyle(FToolMenuEntry& Entry)
	{
		Entry.SubMenuData.Style.StyleName = TEXT("ContentBrowser.AddNewMenu");
	}
}

void FLunarAssetCreationMenu::Register()
{
	RegisterRootMenuIcon();
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLunarAssetCreationMenu::RegisterMenus));
}

void FLunarAssetCreationMenu::Unregister()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	UnregisterRootMenuIcon();
}

void FLunarAssetCreationMenu::RegisterRootMenuIcon()
{
	if (RootMenuIconBrush.IsValid())
	{
		return;
	}

	UTexture2D* IconTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Lunar/Textures/Internal/T_Lunar3.T_Lunar3"));
	if (IconTexture)
	{
		RootMenuIconBrush = FDeferredCleanupSlateBrush::CreateBrush(
			IconTexture,
			FVector2D(20.0, 20.0));
	}
}

void FLunarAssetCreationMenu::UnregisterRootMenuIcon()
{
	RootMenuIconBrush.Reset();
}

void FLunarAssetCreationMenu::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("ContentBrowser.AddNewContextMenu")))
	{
		Menu->AddDynamicSection(
			TEXT("DynamicSection_LunarAssets"),
			FNewToolMenuDelegate::CreateRaw(this, &FLunarAssetCreationMenu::PopulateAddNewMenu));
	}
}

void FLunarAssetCreationMenu::PopulateAddNewMenu(UToolMenu* Menu)
{
	FString PackagePath;
	if (!TryResolvePackagePath(Menu, PackagePath))
	{
		return;
	}

	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("ContentBrowserNewAdvancedAsset"));
	const FText LunarLabel = LOCTEXT("LunarMenuLabel", "Lunar");
	const FText LunarToolTip = LOCTEXT("LunarMenuToolTip", "Create Lunar widgets and UI navigation assets.");
	const FNewToolMenuDelegate PopulateLunarDelegate =
		FNewToolMenuDelegate::CreateRaw(this, &FLunarAssetCreationMenu::PopulateLunarMenu, PackagePath);

	FToolMenuEntry* Entry = nullptr;
	if (RootMenuIconBrush.IsValid())
	{
		Entry = &Section.AddSubMenu(
			TEXT("Lunar"),
			FToolUIActionChoice(),
			SNew(SLunarAssetMenuEntry)
			.Icon(RootMenuIconBrush->GetSlateBrush())
			.Label(LunarLabel)
			.AccentColor(LunarRootMenuAccent)
			.ToolTipText(LunarToolTip),
			PopulateLunarDelegate);
	}
	else
	{
		Entry = &Section.AddSubMenu(
			TEXT("Lunar"),
			LunarLabel,
			LunarToolTip,
			PopulateLunarDelegate);
	}

	Entry->Label = LunarLabel;
	Entry->ToolTip = LunarToolTip;
	Entry->InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
	ApplyContentBrowserMenuStyle(*Entry);
}

void FLunarAssetCreationMenu::PopulateLunarMenu(UToolMenu* Menu, FString PackagePath)
{
	FToolMenuSection& Section = Menu->FindOrAddSection(LunarAssetSectionName);

	FToolMenuEntry& WidgetsEntry = Section.AddSubMenu(
		TEXT("LunarWidgets"),
		LOCTEXT("WidgetsMenuLabel", "Widgets"),
		LOCTEXT("WidgetsMenuToolTip", "Create Lunar Widget Blueprints and reusable widget control classes."),
		FNewToolMenuDelegate::CreateRaw(this, &FLunarAssetCreationMenu::PopulateWidgetMenu, PackagePath));
	ApplyContentBrowserMenuStyle(WidgetsEntry);

	FToolMenuEntry& NavigationEntry = Section.AddSubMenu(
		TEXT("LunarUINavigation"),
		LOCTEXT("NavigationMenuLabel", "UI Navigation"),
		LOCTEXT("NavigationMenuToolTip", "Create Lunar input, prompt, icon, sound-feedback, and haptic-feedback Data Assets."),
		FNewToolMenuDelegate::CreateRaw(this, &FLunarAssetCreationMenu::PopulateNavigationMenu, PackagePath));
	ApplyContentBrowserMenuStyle(NavigationEntry);
}

void FLunarAssetCreationMenu::PopulateWidgetMenu(UToolMenu* Menu, FString PackagePath)
{
	const FName MainSection(TEXT("LunarWidgetsMain"));
	Menu->FindOrAddSection(MainSection, LOCTEXT("MainSectionLabel", "Main"));
	AddWidgetBlueprintEntry(Menu, MainSection, TEXT("LunarScreenWidget"),
		LOCTEXT("ScreenWidgetLabel", "Screen Widget"),
		LOCTEXT("ScreenWidgetToolTip", "Create a top-level Lunar screen with an automatically managed navigation scope."),
		PackagePath, ULunarScreenWidget::StaticClass(), TEXT("W_LunarScreen"));
	AddWidgetBlueprintEntry(Menu, MainSection, TEXT("LunarNavigableWidget"),
		LOCTEXT("NavigableWidgetLabel", "Navigable Widget"),
		LOCTEXT("NavigableWidgetToolTip", "Create a custom Lunar-selectable Widget Blueprint base."),
		PackagePath, ULunarNavigableWidget::StaticClass(), TEXT("W_LunarNavigable"));

	const FName ButtonSection(TEXT("LunarWidgetsButton"));
	Menu->FindOrAddSection(ButtonSection, LOCTEXT("ButtonSectionLabel", "Button"));
	AddWidgetBlueprintEntry(Menu, ButtonSection, TEXT("LunarButton"),
		LOCTEXT("ButtonLabel", "Button"),
		LOCTEXT("ButtonToolTip", "Create a Lunar Button Widget Blueprint."),
		PackagePath, ULunarButton::StaticClass(), TEXT("W_LunarButton"));

	const FName SliderSection(TEXT("LunarWidgetsSlider"));
	Menu->FindOrAddSection(SliderSection, LOCTEXT("SliderSectionLabel", "Slider"));
	AddWidgetBlueprintEntry(Menu, SliderSection, TEXT("LunarSlider"),
		LOCTEXT("SliderLabel", "Slider"),
		LOCTEXT("SliderToolTip", "Create a Lunar Slider Widget Blueprint."),
		PackagePath, ULunarSlider::StaticClass(), TEXT("W_LunarSlider"));

	const FName OptionSliderSection(TEXT("LunarWidgetsOptionSlider"));
	Menu->FindOrAddSection(OptionSliderSection, LOCTEXT("OptionSliderSectionLabel", "Option Slider"));
	AddWidgetBlueprintEntry(Menu, OptionSliderSection, TEXT("LunarOptionSlider"),
		LOCTEXT("OptionSliderLabel", "Option Slider"),
		LOCTEXT("OptionSliderToolTip", "Create a Lunar Option Slider Widget Blueprint."),
		PackagePath, ULunarOptionSlider::StaticClass(), TEXT("W_LunarOptionSlider"));

	const FName SwitchSection(TEXT("LunarWidgetsSwitch"));
	Menu->FindOrAddSection(SwitchSection, LOCTEXT("SwitchSectionLabel", "Switch"));
	AddWidgetBlueprintEntry(Menu, SwitchSection, TEXT("LunarSwitch"),
		LOCTEXT("SwitchLabel", "Switch"),
		LOCTEXT("SwitchToolTip", "Create a Lunar Switch Widget Blueprint."),
		PackagePath, ULunarSwitch::StaticClass(), TEXT("W_LunarSwitch"));

	const FName RadioSection(TEXT("LunarWidgetsRadio"));
	Menu->FindOrAddSection(RadioSection, LOCTEXT("RadioSectionLabel", "Radio"));
	AddWidgetBlueprintEntry(Menu, RadioSection, TEXT("LunarRadio"),
		LOCTEXT("RadioLabel", "Radio"),
		LOCTEXT("RadioToolTip", "Create a composite Lunar Radio Widget Blueprint."),
		PackagePath, ULunarRadio::StaticClass(), TEXT("W_LunarRadio"));
	AddWidgetBlueprintEntry(Menu, RadioSection, TEXT("LunarRadioSideVisual"),
		LOCTEXT("RadioSideVisualLabel", "Radio Side Visual"),
		LOCTEXT("RadioSideVisualToolTip", "Create a non-navigable presentation Widget Blueprint generated beside Lunar Radio options."),
		PackagePath, ULunarRadioSideVisual::StaticClass(), TEXT("W_LunarRadioSideVisual"));

	const FName ScrollBoxSection(TEXT("LunarWidgetsScrollBox"));
	Menu->FindOrAddSection(ScrollBoxSection, LOCTEXT("ScrollBoxSectionLabel", "Scroll Box"));
	AddBlueprintClassEntry(Menu, ScrollBoxSection, TEXT("LunarScrollBox"),
		LOCTEXT("ScrollBoxLabel", "Scroll Box"),
		LOCTEXT("ScrollBoxToolTip", "Create a reusable Blueprint subclass of the Lunar navigation Scroll Box control."),
		PackagePath, ULunarScrollBox::StaticClass(), TEXT("BP_LunarScrollBox"));

	const FName ListViewSection(TEXT("LunarWidgetsListView"));
	Menu->FindOrAddSection(ListViewSection, LOCTEXT("ListViewSectionLabel", "List View"));
	AddWidgetBlueprintEntry(Menu, ListViewSection, TEXT("LunarListView"),
		LOCTEXT("ListViewLabel", "List View"),
		LOCTEXT("ListViewToolTip", "Create a Lunar virtualized List View Widget Blueprint."),
		PackagePath, ULunarListView::StaticClass(), TEXT("W_LunarListView"));
	AddWidgetBlueprintEntry(Menu, ListViewSection, TEXT("LunarListViewEntry"),
		LOCTEXT("ListViewEntryLabel", "List View Entry"),
		LOCTEXT("ListViewEntryToolTip", "Create a non-navigable Blueprint presentation generated for visible Lunar List View rows."),
		PackagePath, ULunarListViewEntry::StaticClass(), TEXT("W_LunarListViewEntry"));

	const FName ComboBoxSection(TEXT("LunarWidgetsComboBox"));
	Menu->FindOrAddSection(ComboBoxSection, LOCTEXT("ComboBoxSectionLabel", "Combo Box"));
	AddWidgetBlueprintEntry(Menu, ComboBoxSection, TEXT("LunarComboBox"),
		LOCTEXT("ComboBoxLabel", "Combo Box"),
		LOCTEXT("ComboBoxToolTip", "Create a Lunar Combo Box Widget Blueprint."),
		PackagePath, ULunarComboBox::StaticClass(), TEXT("W_LunarComboBox"));
	AddWidgetBlueprintEntry(Menu, ComboBoxSection, TEXT("LunarComboBoxSelectedVisual"),
		LOCTEXT("ComboBoxSelectedVisualLabel", "Combo Box Selected Visual"),
		LOCTEXT("ComboBoxSelectedVisualToolTip", "Create the optional closed-face presentation generated by a Lunar Combo Box."),
		PackagePath, ULunarComboBoxSelectedVisual::StaticClass(), TEXT("W_LunarComboBoxSelectedVisual"));
	AddWidgetBlueprintEntry(Menu, ComboBoxSection, TEXT("LunarComboBoxEntry"),
		LOCTEXT("ComboBoxEntryLabel", "Combo Box Entry Visual"),
		LOCTEXT("ComboBoxEntryToolTip", "Create the optional non-navigable presentation generated for visible Lunar Combo Box options."),
		PackagePath, ULunarComboBoxEntry::StaticClass(), TEXT("W_LunarComboBoxEntry"));
	AddWidgetBlueprintEntry(Menu, ComboBoxSection, TEXT("LunarComboBoxEmptyVisual"),
		LOCTEXT("ComboBoxEmptyVisualLabel", "Combo Box Empty Visual"),
		LOCTEXT("ComboBoxEmptyVisualToolTip", "Create the optional empty-filter-results presentation generated by a Lunar Combo Box."),
		PackagePath, ULunarComboBoxEmptyVisual::StaticClass(), TEXT("W_LunarComboBoxEmptyVisual"));

	const FName ContextMenuSection(TEXT("LunarWidgetsContextMenu"));
	Menu->FindOrAddSection(ContextMenuSection, LOCTEXT("ContextMenuSectionLabel", "Context Menu"));
	AddWidgetBlueprintEntry(Menu, ContextMenuSection, TEXT("LunarContextMenu"),
		LOCTEXT("ContextMenuLabel", "Context Menu"),
		LOCTEXT("ContextMenuToolTip", "Create a Lunar Context Menu Widget Blueprint."),
		PackagePath, ULunarContextMenu::StaticClass(), TEXT("W_LunarContextMenu"));

	const FName TabsSection(TEXT("LunarWidgetsTabs"));
	Menu->FindOrAddSection(TabsSection, LOCTEXT("TabsSectionLabel", "Tabs"));
	AddWidgetBlueprintEntry(Menu, TabsSection, TEXT("LunarTabHeader"),
		LOCTEXT("TabHeaderLabel", "Tab Header"),
		LOCTEXT("TabHeaderToolTip", "Create a Lunar Tab Header Widget Blueprint."),
		PackagePath, ULunarTabHeader::StaticClass(), TEXT("W_LunarTabHeader"));
	AddWidgetBlueprintEntry(Menu, TabsSection, TEXT("LunarTabs"),
		LOCTEXT("TabsLabel", "Tabs"),
		LOCTEXT("TabsToolTip", "Create a Lunar Tabs Widget Blueprint."),
		PackagePath, ULunarTabs::StaticClass(), TEXT("W_LunarTabs"));

	const FName InputPromptSection(TEXT("LunarWidgetsInputPrompt"));
	Menu->FindOrAddSection(InputPromptSection, LOCTEXT("InputPromptSectionLabel", "Input Prompt"));
	AddWidgetBlueprintEntry(Menu, InputPromptSection, TEXT("LunarInputPromptWidget"),
		LOCTEXT("InputPromptWidgetLabel", "Input Prompt Widget"),
		LOCTEXT("InputPromptWidgetToolTip", "Create a Lunar input prompt presentation Widget Blueprint."),
		PackagePath, ULunarInputPromptWidget::StaticClass(), TEXT("W_LunarInputPrompt"));
}

void FLunarAssetCreationMenu::PopulateNavigationMenu(UToolMenu* Menu, FString PackagePath)
{
	const FName ActionsSection(TEXT("LunarNavigationActions"));
	Menu->FindOrAddSection(ActionsSection, LOCTEXT("NavigationActionsSectionLabel", "Actions"));
	AddDataAssetEntry(Menu, ActionsSection, TEXT("LunarUIActionRegistry"),
		LOCTEXT("ActionRegistryLabel", "UI Action Registry"),
		LOCTEXT("ActionRegistryToolTip", "Create a registry of semantic UI actions, hotkeys, and prompt display text."),
		PackagePath, ULunarUIActionRegistry::StaticClass(), TEXT("DA_LunarUIActionRegistry"));

	const FName InputPromptsSection(TEXT("LunarNavigationInputPrompts"));
	Menu->FindOrAddSection(InputPromptsSection, LOCTEXT("NavigationInputPromptsSectionLabel", "Input Prompts"));
	AddDataAssetEntry(Menu, InputPromptsSection, TEXT("LunarInputIconSet"),
		LOCTEXT("InputIconSetLabel", "Input Icon Set"),
		LOCTEXT("InputIconSetToolTip", "Create a device-specific mapping from input keys to prompt icons."),
		PackagePath, ULunarInputIconSet::StaticClass(), TEXT("DA_LunarInputIconSet"));

	const FName FeedbackSection(TEXT("LunarNavigationFeedback"));
	Menu->FindOrAddSection(FeedbackSection, LOCTEXT("NavigationFeedbackSectionLabel", "Feedback"));
	AddDataAssetEntry(Menu, FeedbackSection, TEXT("LunarUISoundFeedback"),
		LOCTEXT("SoundFeedbackLabel", "UI Sound Feedback"),
		LOCTEXT("SoundFeedbackToolTip", "Create a reusable full set of Lunar pointer and navigation sounds."),
		PackagePath, ULunarUISoundFeedbackAsset::StaticClass(), TEXT("DA_LunarUISoundFeedback"));
	AddDataAssetEntry(Menu, FeedbackSection, TEXT("LunarUIHapticFeedback"),
		LOCTEXT("HapticFeedbackLabel", "UI Haptic Feedback"),
		LOCTEXT("HapticFeedbackToolTip", "Create a reusable full set of Lunar navigation haptic effects."),
		PackagePath, ULunarUIHapticFeedbackAsset::StaticClass(), TEXT("DA_LunarUIHapticFeedback"));
}

void FLunarAssetCreationMenu::AddWidgetBlueprintEntry(
	UToolMenu* Menu,
	FName SectionName,
	FName EntryName,
	const FText& Label,
	const FText& ToolTip,
	const FString& PackagePath,
	UClass* ParentClass,
	const FString& BaseAssetName)
{
	if (!Menu || !ParentClass)
	{
		return;
	}

	Menu->FindOrAddSection(SectionName).AddMenuEntry(
		EntryName,
		Label,
		ToolTip,
		FSlateIconFinder::FindIconForClass(ParentClass),
		FUIAction(FExecuteAction::CreateStatic(
			&FLunarAssetCreationMenu::CreateWidgetBlueprint,
			PackagePath,
			TWeakObjectPtr<UClass>(ParentClass),
			BaseAssetName)));
}

void FLunarAssetCreationMenu::AddBlueprintClassEntry(
	UToolMenu* Menu,
	FName SectionName,
	FName EntryName,
	const FText& Label,
	const FText& ToolTip,
	const FString& PackagePath,
	UClass* ParentClass,
	const FString& BaseAssetName)
{
	if (!Menu || !ParentClass)
	{
		return;
	}

	Menu->FindOrAddSection(SectionName).AddMenuEntry(
		EntryName,
		Label,
		ToolTip,
		FSlateIconFinder::FindIconForClass(ParentClass),
		FUIAction(FExecuteAction::CreateStatic(
			&FLunarAssetCreationMenu::CreateBlueprintClass,
			PackagePath,
			TWeakObjectPtr<UClass>(ParentClass),
			BaseAssetName)));
}

void FLunarAssetCreationMenu::AddDataAssetEntry(
	UToolMenu* Menu,
	FName SectionName,
	FName EntryName,
	const FText& Label,
	const FText& ToolTip,
	const FString& PackagePath,
	UClass* DataAssetClass,
	const FString& BaseAssetName)
{
	if (!Menu || !DataAssetClass)
	{
		return;
	}

	Menu->FindOrAddSection(SectionName).AddMenuEntry(
		EntryName,
		Label,
		ToolTip,
		FSlateIconFinder::FindIconForClass(DataAssetClass),
		FUIAction(FExecuteAction::CreateStatic(
			&FLunarAssetCreationMenu::CreateDataAsset,
			PackagePath,
			TWeakObjectPtr<UClass>(DataAssetClass),
			BaseAssetName)));
}

void FLunarAssetCreationMenu::CreateWidgetBlueprint(
	FString PackagePath,
	TWeakObjectPtr<UClass> ParentClass,
	FString BaseAssetName)
{
	UClass* ResolvedParentClass = ParentClass.Get();
	if (!ResolvedParentClass || !ResolvedParentClass->IsChildOf(UUserWidget::StaticClass()))
	{
		return;
	}

	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->ParentClass = ResolvedParentClass;
	BeginAssetCreation(PackagePath, BaseAssetName, Factory->GetSupportedClass(), Factory);
}

void FLunarAssetCreationMenu::CreateBlueprintClass(
	FString PackagePath,
	TWeakObjectPtr<UClass> ParentClass,
	FString BaseAssetName)
{
	UClass* ResolvedParentClass = ParentClass.Get();
	if (!ResolvedParentClass)
	{
		return;
	}

	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
	Factory->ParentClass = ResolvedParentClass;
	BeginAssetCreation(PackagePath, BaseAssetName, Factory->GetSupportedClass(), Factory);
}

void FLunarAssetCreationMenu::CreateDataAsset(
	FString PackagePath,
	TWeakObjectPtr<UClass> DataAssetClass,
	FString BaseAssetName)
{
	UClass* ResolvedDataAssetClass = DataAssetClass.Get();
	if (!ResolvedDataAssetClass || !ResolvedDataAssetClass->IsChildOf(UDataAsset::StaticClass()))
	{
		return;
	}

	UDataAssetFactory* Factory = NewObject<UDataAssetFactory>();
	Factory->DataAssetClass = ResolvedDataAssetClass;
	BeginAssetCreation(PackagePath, BaseAssetName, ResolvedDataAssetClass, Factory);
}

void FLunarAssetCreationMenu::BeginAssetCreation(
	const FString& PackagePath,
	const FString& BaseAssetName,
	UClass* AssetClass,
	UFactory* Factory)
{
	if (!FPackageName::IsValidPath(PackagePath) || BaseAssetName.IsEmpty() || !AssetClass || !Factory)
	{
		return;
	}

	FGCObjectScopeGuard FactoryGuard(Factory);
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
	FString UniquePackageName;
	FString UniqueAssetName;
	AssetTools.CreateUniqueAssetName(
		FString::Printf(TEXT("%s/%s"), *PackagePath, *BaseAssetName),
		FString(),
		UniquePackageName,
		UniqueAssetName);

	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.Get().CreateNewAsset(UniqueAssetName, PackagePath, AssetClass, Factory);
}

bool FLunarAssetCreationMenu::TryResolvePackagePath(const UToolMenu* Menu, FString& OutPackagePath)
{
	OutPackagePath.Reset();
	if (!Menu)
	{
		return false;
	}

	const UContentBrowserDataMenuContext_AddNewMenu* Context =
		Menu->FindContext<UContentBrowserDataMenuContext_AddNewMenu>();
	if (!Context || !Context->bCanBeModified || !Context->bContainsValidPackagePath || Context->SelectedPaths.Num() != 1)
	{
		return false;
	}

	FName InternalPath;
	UContentBrowserDataSubsystem* ContentBrowserData = IContentBrowserDataModule::Get().GetSubsystem();
	if (!ContentBrowserData ||
		ContentBrowserData->TryConvertVirtualPath(Context->SelectedPaths[0], InternalPath) != EContentBrowserPathType::Internal)
	{
		return false;
	}

	OutPackagePath = InternalPath.ToString();
	if (!FPackageName::IsValidPath(OutPackagePath))
	{
		OutPackagePath.Reset();
		return false;
	}

	const TArray<FString> PackagePaths = {OutPackagePath};
	const IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
	if (!AssetTools.AllPassWritableFolderFilter(PackagePaths))
	{
		OutPackagePath.Reset();
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
