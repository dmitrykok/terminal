// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

#include "NewProjectMenu.g.h"
#include "NewProjectMenuEntryTemplateSelector.g.h"
#include "Utils.h"

namespace winrt::Microsoft::Terminal::Settings::Editor::implementation
{
    struct NewProjectMenu : public HasScrollViewer<NewProjectMenu>, NewProjectMenuT<NewProjectMenu>
    {
    public:
        NewProjectMenu();

        void OnNavigatedTo(const winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs& e);

        // FolderPickerDialog handlers
        void ProjectFolderPickerDialog_Opened(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs& e);
        void ProjectFolderPickerDialog_PrimaryButtonClick(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs& e);

        // NTM Entry handlers
        void ProjectEditEntry_Clicked(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);
        void ProjectReorderEntry_Clicked(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);
        void ProjectDeleteEntry_Clicked(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);

        // Multiple Entry handlers
        safe_void_coroutine ProjectMoveMultiple_Click(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);
        void ProjectDeleteMultiple_Click(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);

        // New Entry handlers
        void AddProjectButton_Clicked(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);
        void AddProjectSeparatorButton_Clicked(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);
        void AddProjectFolderButton_Clicked(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);
        void AddProjectMatchButton_Clicked(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);
        void AddProjectRemainingButton_Clicked(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::RoutedEventArgs& e);
        void AddProjectFolderNameTextBox_KeyDown(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs& e);
        void AddProjectFolderNameTextBox_TextChanged(const winrt::Windows::Foundation::IInspectable& sender, const winrt::Windows::UI::Xaml::Controls::TextChangedEventArgs& e);

        WINRT_CALLBACK(PropertyChanged, Windows::UI::Xaml::Data::PropertyChangedEventHandler);
        WINRT_OBSERVABLE_PROPERTY(Editor::NewProjectMenuViewModel, ViewModel, _PropertyChangedHandlers, nullptr);

    private:
        Editor::NewProjectMenuEntryViewModel _draggedEntry{ nullptr };

        void _ScrollToEntry(const Editor::NewProjectMenuEntryViewModel& entry);
    };

    struct NewProjectMenuEntryTemplateSelector : public NewProjectMenuEntryTemplateSelectorT<NewProjectMenuEntryTemplateSelector>
    {
    public:
        NewProjectMenuEntryTemplateSelector() = default;

        Windows::UI::Xaml::DataTemplate SelectTemplateCore(const Windows::Foundation::IInspectable& item, const Windows::UI::Xaml::DependencyObject& container);
        Windows::UI::Xaml::DataTemplate SelectTemplateCore(const Windows::Foundation::IInspectable& item);

        WINRT_PROPERTY(Windows::UI::Xaml::DataTemplate, ProjectEntryTemplate, nullptr);
        WINRT_PROPERTY(Windows::UI::Xaml::DataTemplate, ProjectActionEntryTemplate, nullptr);
        WINRT_PROPERTY(Windows::UI::Xaml::DataTemplate, ProjectSeparatorEntryTemplate, nullptr);
        WINRT_PROPERTY(Windows::UI::Xaml::DataTemplate, ProjectFolderEntryTemplate, nullptr);
        WINRT_PROPERTY(Windows::UI::Xaml::DataTemplate, ProjectMatchEntryTemplate, nullptr);
        WINRT_PROPERTY(Windows::UI::Xaml::DataTemplate, ProjectRemainingEntryTemplate, nullptr);
    };
}

namespace winrt::Microsoft::Terminal::Settings::Editor::factory_implementation
{
    BASIC_FACTORY(NewProjectMenu);
    BASIC_FACTORY(NewProjectMenuEntryTemplateSelector);
}
