# QuantaOccipita.ps1
# Description: Bootstraps a documentation folder for the next QuantaSoft module

# Script Version
$scriptVersion = "1.0.0"

param(
    [string]$ModuleName,
    [switch]$NoTests,
    [switch]$DryRun,
    [switch]$Force,
    [switch]$Version,
    [switch]$Help,
    [switch]$Quiet
)

# Helper function for quiet mode
function Write-Output-Quiet {
    param (
        [string]$Message
    )
    if (-not $Quiet) {
        Write-Host $Message
    }
}

# Help Message
$helpMessage = @"
QuantaOccipita - QuantaSoft Module Bootstrapper v$($scriptVersion)

Usage: ./QuantaOccipita.ps1 [options]

Description:
  This script bootstraps a new QuantaSoft module by generating a standard
  directory structure, boilerplate code, and documentation.

Options:
  -ModuleName <string>  Specifies the name of the module to be created.
                        If not provided, the script will prompt for it.

  -NoTests              Skips the creation of the 'test/' directory and
                        unit test skeletons.

  -DryRun               Displays a summary of the files and directories
                        that would be created without actually creating them.

  -Force                Overwrites existing files ('plan.md', 'enhancements.md')
                        if they already exist.

  -Version              Displays the current version of the script.

  -Help                 Displays this help message.

Example:
  ./QuantaOccipita.ps1 -ModuleName "QuantaCortex" -NoTests
"@

if ($Help) {
    Write-Output-Quiet $helpMessage
    exit
}

if ($Version) {
    Write-Output-Quiet $scriptVersion
    exit
}

if (-not $ModuleName) {
    $ModuleName = Read-Host "Enter the name for the new module"
}

# Dependency Check
$gitCheck = Get-Command git -ErrorAction SilentlyContinue
if (-not $gitCheck) {
    Write-Error "Git is not installed or not in your PATH. Please install Git to continue."
    exit 1
}

$docsPath = Join-Path (Get-Location) "docs"

if ($DryRun) {
    Write-Output-Quiet "--- Dry Run Mode ---"
    Write-Output-Quiet "Module Name: $ModuleName"
    Write-Output-Quiet "Create Tests: !$NoTests"
    Write-Output-Quiet "Force Overwrite: $Force"
    Write-Output-Quiet "--------------------"
}

# Create docs directory if not exists
if (-Not (Test-Path $docsPath)) {
    if (-not $DryRun) {
        New-Item -Path $docsPath -ItemType Directory
    }
    Write-Output-Quiet "Created docs directory at $docsPath"
} else {
    Write-Output-Quiet "docs directory already exists at $docsPath"
}

# plan.md content
$planContent = @"
# 📘 Module Planning Document

## Module Name
- Placeholder: `Quanta[$ModuleName]` (e.g., QuantaCortex, QuantaThalamus)

## Purpose
- Define the purpose of this module in the QuantaSoft system.
- What role does it play within the agent ecosystem?

## Goals
- [ ] Define inputs and outputs
- [ ] Define triggers and dependencies
- [ ] Establish ethical decision logic (if applicable)

## Integration Plan
- How will this module communicate with:
  - 🧠 QuantaSensa
  - 🕊️ QuantaEthos
  - 🧬 QuantaSynapse
  - 🧹 QuantaGlia

## Notes
- Anticipated constraints:
- Potential edge cases:
"@

# enhancements.md content
$enhancementsContent = @"
# 🌱 Module Enhancement Opportunities

## UX or CLI Improvements
- [ ] Streamline feedback or log formats
- [ ] Add verbosity toggles or progress display

## Learning & Adaptation
- [ ] Integrate new pattern recognition or behavior shaping logic
- [ ] Enable module to update itself from verified QuantaGlia knowledge

## Resilience Features
- [ ] Better handling of low-resource environments
- [ ] Soft fallback routines when encountering unknown inputs

## Interoperability
- [ ] Enable tighter scheduling control via QuantaParent
- [ ] Improve YAML or JSON config readability

## Future Roadmap
- [ ] Federated integration across machines?
- [ ] Test in embedded or containerized environments
"@

# Write files
$planPath = Join-Path $docsPath "plan.md"
$enhancementsPath = Join-Path $docsPath "enhancements.md"

if ($Force -or -not (Test-Path $planPath)) {
    if (-not $DryRun) {
        Set-Content -Path $planPath -Value $planContent
    }
    Write-Output-Quiet "Generated plan.md with placeholders in $docsPath"
} else {
    Write-Output-Quiet "plan.md already exists. Use --force to overwrite."
}

if ($Force -or -not (Test-Path $enhancementsPath)) {
    if (-not $DryRun) {
        Set-Content -Path $enhancementsPath -Value $enhancementsContent
    }
    Write-Output-Quiet "Generated enhancements.md with placeholders in $docsPath"
} else {
    Write-Output-Quiet "enhancements.md already exists. Use --force to overwrite."
}

# Unit Test Skeletons
if (-not $NoTests) {
    $createTests = $true
    if ($PSBoundParameters.ContainsKey('NoTests') -eq $false) {
        $response = Read-Host "Create unit test skeletons? (Y/N)"
        if ($response -ne 'Y' -and $response -ne 'y') {
            $createTests = $false
        }
    }

    if ($createTests) {
        $testPath = Join-Path (Get-Location) "test"
        if (-Not (Test-Path $testPath)) {
            if (-not $DryRun) {
                New-Item -Path $testPath -ItemType Directory
            }
            Write-Output-Quiet "Created test directory at $testPath"
        } else {
            Write-Output-Quiet "test directory already exists at $testPath"
        }
    }
}
