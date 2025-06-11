// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

#include "NewProjectMenuViewModel.g.h"
#include "ProjectFolderTreeViewEntry.g.h"
#include "NewProjectMenuEntryViewModel.g.h"
#include "ProjectEntryViewModel.g.h"
#include "ProjectActionEntryViewModel.g.h"
#include "ProjectSeparatorEntryViewModel.g.h"
#include "ProjectFolderEntryViewModel.g.h"
#include "ProjectMatchEntryViewModel.g.h"
#include "ProjectRemainingEntryViewModel.g.h"

//#include "ProjectViewModel.h"
#include "ViewModelHelpers.h"
#include "Utils.h"

namespace winrt::Microsoft::Terminal::Settings::Editor::implementation
{
    struct NewProjectMenuViewModel : NewProjectMenuViewModelT<NewProjectMenuViewModel>, ViewModelHelper<NewProjectMenuViewModel>
    {
    public:
        NewProjectMenuViewModel(Model::CascadiaSettings settings);
        void UpdateSettings(const Model::CascadiaSettings& settings);
        void GenerateFolderTree();
        Windows::Foundation::Collections::IVector<Editor::ProjectFolderEntryViewModel> FindFolderPathByName(const hstring& name);

        bool IsProjectRemainingEntryMissing() const;
        bool IsProjectFolderView() const noexcept;

        void RequestReorderEntry(const Editor::NewProjectMenuEntryViewModel& vm, bool goingUp);
        void RequestDeleteEntry(const Editor::NewProjectMenuEntryViewModel& vm);
        void RequestMoveEntriesToFolder(const Windows::Foundation::Collections::IVector<Editor::NewProjectMenuEntryViewModel>& entries, const Editor::ProjectFolderEntryViewModel& destinationFolder);

        Editor::NewProjectMenuEntryViewModel RequestAddSelectedProjectEntry();
        Editor::NewProjectMenuEntryViewModel RequestAddProjectSeparatorEntry();
        Editor::NewProjectMenuEntryViewModel RequestAddProjectFolderEntry();
        Editor::NewProjectMenuEntryViewModel RequestAddProjectMatcherEntry();
        Editor::NewProjectMenuEntryViewModel RequestAddProjectRemainingEntry();

        hstring CurrentFolderName() const;
        void CurrentFolderName(const hstring& value);
        bool CurrentFolderInlining() const;
        void CurrentFolderInlining(bool value);
        bool CurrentFolderAllowEmpty() const;
        void CurrentFolderAllowEmpty(bool value);

        // TODO: remove return nullptr
        Windows::Foundation::Collections::IObservableVector<Model::Project> AvailableProjects() const { return nullptr; }
        Windows::Foundation::Collections::IObservableVector<Editor::ProjectFolderTreeViewEntry> FolderTree() const;
        Windows::Foundation::Collections::IObservableVector<Editor::NewProjectMenuEntryViewModel> ProjectCurrentView() const;
        VIEW_MODEL_OBSERVABLE_PROPERTY(Editor::ProjectFolderEntryViewModel, CurrentProjectFolder, nullptr);
        VIEW_MODEL_OBSERVABLE_PROPERTY(Editor::ProjectFolderTreeViewEntry, CurrentProjectFolderTreeViewSelectedItem, nullptr);

        // Bound to the UI to create new entries
        VIEW_MODEL_OBSERVABLE_PROPERTY(Model::Project, SelectedProject, nullptr);
        VIEW_MODEL_OBSERVABLE_PROPERTY(hstring, ProjectMatcherName);
        VIEW_MODEL_OBSERVABLE_PROPERTY(hstring, ProjectMatcherSource);
        VIEW_MODEL_OBSERVABLE_PROPERTY(hstring, ProjectMatcherCommandline);
        VIEW_MODEL_OBSERVABLE_PROPERTY(hstring, AddProjectFolderName);

    private:
        Model::CascadiaSettings _Settings{ nullptr };
        Windows::Foundation::Collections::IObservableVector<Editor::NewProjectMenuEntryViewModel> _rootEntries;
        Windows::Foundation::Collections::IObservableVector<Editor::ProjectFolderTreeViewEntry> _folderTreeCache;
        Windows::Foundation::Collections::IObservableVector<Editor::NewProjectMenuEntryViewModel>::VectorChanged_revoker _rootEntriesChangedRevoker;

        static bool _IsProjectRemainingEntryMissing(const Windows::Foundation::Collections::IVector<Editor::NewProjectMenuEntryViewModel>& entries);
        void _FolderPropertyChanged(const IInspectable& sender, const Windows::UI::Xaml::Data::PropertyChangedEventArgs& args);
    };

    struct ProjectFolderTreeViewEntry : ProjectFolderTreeViewEntryT<ProjectFolderTreeViewEntry>
    {
    public:
        ProjectFolderTreeViewEntry(Editor::ProjectFolderEntryViewModel folderEntry);

        hstring Name() const;
        hstring Icon() const;
        Editor::ProjectFolderEntryViewModel FolderEntryVM() { return _folderEntry; }

        WINRT_PROPERTY(Windows::Foundation::Collections::IObservableVector<Microsoft::Terminal::Settings::Editor::ProjectFolderTreeViewEntry>, Children);

    private:
        Editor::ProjectFolderEntryViewModel _folderEntry;
    };

    struct NewProjectMenuEntryViewModel : NewProjectMenuEntryViewModelT<NewProjectMenuEntryViewModel>, ViewModelHelper<NewProjectMenuEntryViewModel>
    {
    public:
        static Model::NewProjectMenuEntry GetModel(const Editor::NewProjectMenuEntryViewModel& viewModel);
        VIEW_MODEL_OBSERVABLE_PROPERTY(Model::NewProjectMenuEntryType, Type, Model::NewProjectMenuEntryType::Invalid);

    protected:
        explicit NewProjectMenuEntryViewModel(const Model::NewProjectMenuEntryType type) noexcept;
    };

    struct ProjectEntryViewModel : ProjectEntryViewModelT<ProjectEntryViewModel, NewProjectMenuEntryViewModel>
    {
    public:
        ProjectEntryViewModel(Model::ProjectEntry projectEntry);

        VIEW_MODEL_OBSERVABLE_PROPERTY(Model::ProjectEntry, ProjectEntry, nullptr);
    };

    struct ProjectActionEntryViewModel : ProjectActionEntryViewModelT<ProjectActionEntryViewModel, NewProjectMenuEntryViewModel>
    {
    public:
        ProjectActionEntryViewModel(Model::ProjectActionEntry actionEntry, Model::CascadiaSettings settings);

        hstring DisplayText() const;
        hstring Icon() const;
        VIEW_MODEL_OBSERVABLE_PROPERTY(Model::ProjectActionEntry, ProjectActionEntry, nullptr);

    private:
        Model::CascadiaSettings _Settings;
    };

    struct ProjectSeparatorEntryViewModel : ProjectSeparatorEntryViewModelT<ProjectSeparatorEntryViewModel, NewProjectMenuEntryViewModel>
    {
    public:
        ProjectSeparatorEntryViewModel(Model::ProjectSeparatorEntry separatorEntry);

        VIEW_MODEL_OBSERVABLE_PROPERTY(Model::ProjectSeparatorEntry, ProjectSeparatorEntry, nullptr);
    };

    struct ProjectFolderEntryViewModel : ProjectFolderEntryViewModelT<ProjectFolderEntryViewModel, NewProjectMenuEntryViewModel>
    {
    public:
        ProjectFolderEntryViewModel(Model::ProjectFolderEntry folderEntry, Model::CascadiaSettings settings);
        explicit ProjectFolderEntryViewModel(Model::ProjectFolderEntry folderEntry);

        bool Inlining() const;
        void Inlining(bool value);
        GETSET_OBSERVABLE_PROJECTED_SETTING(_ProjectFolderEntry, Name);
        GETSET_OBSERVABLE_PROJECTED_SETTING(_ProjectFolderEntry, Icon);
        GETSET_OBSERVABLE_PROJECTED_SETTING(_ProjectFolderEntry, AllowEmpty);

        VIEW_MODEL_OBSERVABLE_PROPERTY(Windows::Foundation::Collections::IObservableVector<Editor::NewProjectMenuEntryViewModel>, Entries);
        VIEW_MODEL_OBSERVABLE_PROPERTY(Model::ProjectFolderEntry, ProjectFolderEntry, nullptr);

    private:
        Windows::Foundation::Collections::IObservableVector<Editor::NewProjectMenuEntryViewModel>::VectorChanged_revoker _entriesChangedRevoker;
        Model::CascadiaSettings _Settings;
    };

    struct ProjectMatchEntryViewModel : ProjectMatchEntryViewModelT<ProjectMatchEntryViewModel, NewProjectMenuEntryViewModel>
    {
    public:
        ProjectMatchEntryViewModel(Model::ProjectMatchEntry projectMatchEntry);

        hstring DisplayText() const;
        VIEW_MODEL_OBSERVABLE_PROPERTY(Model::ProjectMatchEntry, ProjectMatchEntry, nullptr);
    };

    struct ProjectRemainingEntryViewModel : ProjectRemainingEntryViewModelT<ProjectRemainingEntryViewModel, NewProjectMenuEntryViewModel>
    {
    public:
        ProjectRemainingEntryViewModel(Model::ProjectRemainingEntry projectRemainingEntry);

        VIEW_MODEL_OBSERVABLE_PROPERTY(Model::ProjectRemainingEntry, ProjectRemainingEntry, nullptr);
    };
};

namespace winrt::Microsoft::Terminal::Settings::Editor::factory_implementation
{
    BASIC_FACTORY(NewProjectMenuViewModel);
    BASIC_FACTORY(ProjectFolderTreeViewEntry);
    BASIC_FACTORY(ProjectEntryViewModel);
    BASIC_FACTORY(ProjectActionEntryViewModel);
    BASIC_FACTORY(ProjectSeparatorEntryViewModel);
    BASIC_FACTORY(ProjectFolderEntryViewModel);
    BASIC_FACTORY(ProjectMatchEntryViewModel);
    BASIC_FACTORY(ProjectRemainingEntryViewModel);
}
