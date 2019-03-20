package VSObjectFactory;

#
# Package that creates Visual Studio wrapper objects for msvc build
#
# src/tools/msvc/VSObjectFactory.pm
#

use Carp;
use strict;
use warnings;

use Exporter;
use Project;
use Solution;
use MSBuildProject;

our (@ISA, @EXPORT);
@ISA    = qw(Exporter);
@EXPORT = qw(CreateSolution CreateProject DetermineVisualStudioVersion);

no warnings qw(redefine);    ## no critic

sub CreateSolution
{
	my $visualStudioVersion = shift;

	if (!defined($visualStudioVersion))
	{
		$visualStudioVersion = DetermineVisualStudioVersion();
	}

	if ($visualStudioVersion eq '12.00')
	{
		return new VS2013Solution(@_);
	}
	elsif ($visualStudioVersion eq '14.00')
	{
		return new VS2015Solution(@_);
	}

	# visual studio 2017 nmake version is greather than 14.10 and less than 14.20.
	# but the version number is 15.00
	# so adjust the check to support it.
	elsif ((($visualStudioVersion ge '14.10')
		and ($visualStudioVersion lt '14.20'))
		or ($visualStudioVersion eq '15.00'))
	{
		return new VS2017Solution(@_);
	}

	# visual studio 2019 nmake version is greather than 14.20 and less than 14.30 (expected).
	# but the version number is 16.00
	# so adjust the check to support it.
	elsif ((($visualStudioVersion ge '14.20')
		and ($visualStudioVersion lt '14.30'))
		or ($visualStudioVersion eq '16.00'))
	{
		return new VS2019Solution(@_);
	}
	else
	{
		carp $visualStudioVersion;
		croak "The requested Visual Studio version is not supported.";
	}
}

sub CreateProject
{
	my $visualStudioVersion = shift;

	if (!defined($visualStudioVersion))
	{
		$visualStudioVersion = DetermineVisualStudioVersion();
	}

	if ($visualStudioVersion eq '12.00')
	{
		return new VC2013Project(@_);
	}
	elsif ($visualStudioVersion eq '14.00')
	{
		return new VC2015Project(@_);
	}

	# visual studio 2017 nmake version is greather than 14.10 and less than 14.20.
	# but the version number is 15.00
	# so adjust the check to support it.
	elsif ((($visualStudioVersion ge '14.10')
		and ($visualStudioVersion lt '14.20'))
		or ($visualStudioVersion eq '15.00'))
	{
		return new VC2017Project(@_);
	}

	# visual studio 2019 nmake version is greather than 14.20 and less than 14.30 (expected).
	# but the version number is 16.00
	# so adjust the check to support it.
	elsif ((($visualStudioVersion ge '14.20')
		and ($visualStudioVersion lt '14.30'))
		or ($visualStudioVersion eq '16.00'))
	{
		return new VC2019Project(@_);
	}
	else
	{
		carp $visualStudioVersion;
		croak "The requested Visual Studio version is not supported.";
	}
}

sub DetermineVisualStudioVersion
{

	# To determine version of Visual Studio we use nmake as it has
	# existed for a long time and still exists in current Visual
	# Studio versions.
	my $output = `nmake /? 2>&1`;
	$? >> 8 == 0
	  or croak
	  "Unable to determine Visual Studio version: The nmake command wasn't found.";
	if ($output =~ /(\d+)\.(\d+)\.\d+(\.\d+)?$/m)
	{
		return _GetVisualStudioVersion($1, $2);
	}

	croak
	  "Unable to determine Visual Studio version: The nmake version could not be determined.";
}

sub _GetVisualStudioVersion
{
	my ($major, $minor) = @_;

	# The major visual stuido that is suppored has nmake version >= 14.20 and < 15.
	if ($major > 14)
	{
		carp
		  "The determined version of Visual Studio is newer than the latest supported version. Returning the latest supported version instead.";
		return '14.20';
	}
	elsif ($major < 12)
	{
		croak
		  "Unable to determine Visual Studio version: Visual Studio versions before 12.0 aren't supported.";
	}
	return "$major.$minor";
}

1;
