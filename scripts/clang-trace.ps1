function Build-Once {
    ninja clean

    $sw = [Diagnostics.Stopwatch]::StartNew()
    ninja
    $sw.Stop()
    $duration = $sw.Elapsed.TotalSeconds
    Write-Host "Build done in $duration seconds"
}

Push-Location ..
cmake -B build/trace -DDEMO_CLANG_TRACE=1 -GNinja
Push-Location build/trace

Get-ChildItem *.json -Recurse | ForEach-Object { Remove-Item -Path $_.FullName }

Build-Once
Build-Once
Build-Once

ClangBuildAnalyzer --all . capture
ClangBuildAnalyzer --analyze capture > capture.txt

Write-Host "Results written to $((Get-ChildItem capture.txt).FullName)"

Pop-Location
Pop-Location
