// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "NewProjectMenu.h"
#include "NewProjectMenu.g.cpp"
#include "NewProjectMenuEntryTemplateSelector.g.cpp"
#include "EnumEntry.h"

#include "NewProjectMenuViewModel.h"

#include <LibraryResources.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::Terminal::Settings::Model;

namespace winrt::Microsoft::Terminal::Settings::Editor::implementation
{
    NewProjectMenu::NewProjectMenu()
    {
        InitializeComponent();

        // Ideally, we'd bind IsEnabled to something like mtu:Converters.isEmpty(NewProjectMenuListView.SelectedItems.Size) in the XAML,
        //   but the XAML compiler can't find NewProjectMenuListView when we try that. Rather than copying the list of selected items over
        //   to the view model, we'll just do this instead (much simpler).
        NewProjectMenuListView().SelectionChanged([this](auto&&, auto&&) {
            const auto list = NewProjectMenuListView();
            MoveToFolderButton().IsEnabled(list.SelectedItems().Size() > 0);
            DeleteMultipleButton().IsEnabled(list.SelectedItems().Size() > 0);
        });

        Automation::AutomationProperties::SetName(ProjectMoveToFolderButton(), RS_(L"NewProjectMenu_MoveToFolderTextBlock/Text"));
        Automation::AutomationProperties::SetName(ProjectDeleteMultipleButton(), RS_(L"NewProjectMenu_DeleteMultipleTextBlock/Text"));
        Automation::AutomationProperties::SetName(AddProjectComboBox(), RS_(L"NewProjectMenu_AddProject/Header"));
        Automation::AutomationProperties::SetName(AddProjectButton(), RS_(L"NewProjectMenu_AddProjectButton/[using:Windows.UI.Xaml.Controls]ToolTipService/ToolTip"));
        Automation::AutomationProperties::SetName(AddProjectSeparatorButton(), RS_(L"NewProjectMenu_AddSeparatorButton/[using:Windows.UI.Xaml.Controls]ToolTipService/ToolTip"));
        Automation::AutomationProperties::SetName(AddProjectFolderButton(), RS_(L"NewProjectMenu_AddFolderButton/[using:Windows.UI.Xaml.Controls]ToolTipService/ToolTip"));
        Automation::AutomationProperties::SetName(AddProjectMatchButton(), RS_(L"NewProjectMenu_AddProjectMatchTextBlock/Text"));
        Automation::AutomationProperties::SetName(AddProjectRemainingButton(), RS_(L"NewProjectMenu_AddProjectRemainingButton/[using:Windows.UI.Xaml.Controls]ToolTipService/ToolTip"));
    }

    void NewProjectMenu::OnNavigatedTo(const NavigationEventArgs& e)
    {
        _ViewModel = e.Parameter().as<Editor::NewProjectMenuViewModel>();
    }

    void NewProjectMenu::ProjectFolderPickerDialog_Opened(const IInspectable& /*sender*/, const Controls::ContentDialogOpenedEventArgs& /*e*/)
    {
        // Ideally, we'd bind IsPrimaryButtonEnabled to something like mtu:Converters.isEmpty(FolderTree.SelectedItems.Size) in the XAML.
        // Similar to above, the XAML compiler can't find FolderTree when we try that.
        // To make matters worse, SelectionChanged doesn't exist for WinUI 2's TreeView.
        // Let's just select the first item and call it a day.
        _ViewModel.GenerateFolderTree();
        _ViewModel.CurrentProjectFolderTreeViewSelectedItem(_ViewModel.FolderTree().First().Current());
    }

    void NewProjectMenu::ProjectFolderPickerDialog_PrimaryButtonClick(const IInspectable& /*sender*/, const Controls::ContentDialogButtonClickEventArgs& /*e*/)
    {
        // copy selected items first (it updates as we move entries)
        std::vector<Editor::NewProjectMenuEntryViewModel> entries;
        for (const auto& item : NewProjectMenuListView().SelectedItems())
        {
            entries.push_back(item.as<Editor::NewProjectMenuEntryViewModel>());
        }

        // now actually move them
        _ViewModel.RequestMoveEntriesToFolder(single_threaded_vector<Editor::NewProjectMenuEntryViewModel>(std::move(entries)), ProjectFolderTreeView().SelectedItem().as<Editor::FolderTreeViewEntry>().FolderEntryVM());
    }

    void NewProjectMenu::ProjectEditEntry_Clicked(const IInspectable& sender, const RoutedEventArgs& /*e*/)
    {
        const auto folderVM = sender.as<FrameworkElement>().DataContext().as<Editor::ProjectFolderEntryViewModel>();
        _ViewModel.CurrentFolder(folderVM);
    }

    void NewProjectMenu::ProjectReorderEntry_Clicked(const IInspectable& sender, const RoutedEventArgs& /*e*/)
    {
        const auto btn = sender.as<Controls::Button>();
        const auto entry = btn.DataContext().as<Editor::NewProjectMenuEntryViewModel>();
        const auto direction = unbox_value<hstring>(btn.Tag());

        _ViewModel.RequestReorderEntry(entry, direction == L"Up");
    }

    void NewProjectMenu::ProjectDeleteEntry_Clicked(const IInspectable& sender, const RoutedEventArgs& /*e*/)
    {
        const auto entry = sender.as<Controls::Button>().DataContext().as<Editor::NewProjectMenuEntryViewModel>();
        _ViewModel.RequestDeleteEntry(entry);
    }

    safe_void_coroutine NewProjectMenu::ProjectMoveMultiple_Click(const IInspectable& /*sender*/, const RoutedEventArgs& /*e*/)
    {
        co_await FindName(L"FolderPickerDialog").as<Controls::ContentDialog>().ShowAsync();
    }

    void NewProjectMenu::ProjectDeleteMultiple_Click(const IInspectable& /*sender*/, const RoutedEventArgs& /*e*/)
    {
        // copy selected items first (it updates as we delete entries)
        std::vector<Editor::NewProjectMenuEntryViewModel> entries;
        for (const auto& item : NewProjectMenuListView().SelectedItems())
        {
            entries.push_back(item.as<Editor::NewProjectMenuEntryViewModel>());
        }

        // now actually delete them
        for (const auto& vm : entries)
        {
            _ViewModel.RequestDeleteEntry(vm);
        }
    }

    void NewProjectMenu::AddProjectButton_Clicked(const IInspectable& /*sender*/, const RoutedEventArgs& /*e*/)
    {
        _ScrollToEntry(_ViewModel.RequestAddSelectedProjectEntry());
    }

    void NewProjectMenu::AddSeparatorButton_Clicked(const IInspectable& /*sender*/, const RoutedEventArgs& /*e*/)
    {
        _ScrollToEntry(_ViewModel.RequestAddSeparatorEntry());
    }

    void NewProjectMenu::AddFolderButton_Clicked(const IInspectable& /*sender*/, const RoutedEventArgs& /*e*/)
    {
        _ScrollToEntry(_ViewModel.RequestAddFolderEntry());
    }

    void NewProjectMenu::AddMatchProjectsButton_Clicked(const IInspectable& /*sender*/, const RoutedEventArgs& /*e*/)
    {
        _ScrollToEntry(_ViewModel.RequestAddProjectMatcherEntry());
    }

    void NewProjectMenu::AddRemainingProjectsButton_Clicked(const IInspectable& /*sender*/, const RoutedEventArgs& /*e*/)
    {
        _ScrollToEntry(_ViewModel.RequestAddRemainingProjectsEntry());
    }

    // As a QOL improvement, we scroll to the newly added entry.
    // Calling ScrollIntoView() on its own causes the items to briefly disappear.
    // Calling UpdateLayout() beforehand remedies this issue.
    void NewProjectMenu::_ScrollToEntry(const Editor::NewProjectMenuEntryViewModel& entry)
    {
        const auto& listView = NewProjectMenuListView();
        listView.UpdateLayout();
        listView.ScrollIntoView(entry);
    }

    void NewProjectMenu::AddFolderNameTextBox_KeyDown(const IInspectable& /*sender*/, const Input::KeyRoutedEventArgs& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Enter)
        {
            // We need to manually set the FolderName here because the TextBox's TextChanged event hasn't fired yet.
            if (const auto folderName = FolderNameTextBox().Text(); !folderName.empty())
            {
                _ViewModel.AddFolderName(folderName);
                const auto entry = _ViewModel.RequestAddFolderEntry();
                NewProjectMenuListView().ScrollIntoView(entry);
            }
        }
    }

    void NewProjectMenu::AddProjectFolderNameTextBox_TextChanged(const IInspectable& sender, const Controls::TextChangedEventArgs& /*e*/)
    {
        const auto isTextEmpty = sender.as<Controls::TextBox>().Text().empty();
        AddProjectFolderButton().IsEnabled(!isTextEmpty);
    }

    DataTemplate NewProjectMenuEntryTemplateSelector::SelectTemplateCore(const IInspectable& item, const DependencyObject& /*container*/)
    {
        return SelectTemplateCore(item);
    }

    DataTemplate NewProjectMenuEntryTemplateSelector::SelectTemplateCore(const IInspectable& item)
    {
        if (const auto entryVM = item.try_as<Editor::NewProjectMenuEntryViewModel>())
        {
            switch (entryVM.Type())
            {
            case Model::NewProjectMenuEntryType::Project:
                return ProjectEntryTemplate();
            case Model::NewProjectMenuEntryType::ProjectAction:
                return ProjectActionEntryTemplate();
            case Model::NewProjectMenuEntryType::ProjectSeparator:
                return ProjectSeparatorEntryTemplate();
            case Model::NewProjectMenuEntryType::ProjectFolder:
                return ProjectFolderEntryTemplate();
            case Model::NewProjectMenuEntryType::ProjectMatch:
                return ProjectMatchEntryTemplate();
            case Model::NewProjectMenuEntryType::ProjectRemaining:
                return ProjectRemainingEntryTemplate();
            case Model::NewProjectMenuEntryType::Invalid:
            default:
                assert(false);
                return nullptr;
            }
        }
        assert(false);
        return nullptr;
    }
}
