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
use VCBuildProject;
use MSBuildProject;

our (@ISA, @EXPORT);
@ISA    = qw(Exporter);
@EXPORT = qw(CreateSolution CreateProject DetermineVisualStudioVersion);

sub CreateSolution
{
	my $visualStudioVersion = shift;

	if (!defined($visualStudioVersion))
	{
		$visualStudioVersion = DetermineVisualStudioVersion();
	}

	if ($visualStudioVersion eq '8.00')
	{
		return new VS2005Solution(@_);
	}
	elsif ($visualStudioVersion eq '9.00')
	{
		return new VS2008Solution(@_);
	}
	elsif ($visualStudioVersion eq '10.00')
	{
		return new VS2010Solution(@_);
	}
	elsif ($visualStudioVersion eq '11.00')
	{
		return new VS2012Solution(@_);
	}
	elsif ($visualStudioVersion eq '12.00')
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

	if ($visualStudioVersion eq '8.00')
	{
		return new VC2005Project(@_);
	}
	elsif ($visualStudioVersion eq '9.00')
	{
		return new VC2008Project(@_);
	}
	elsif ($visualStudioVersion eq '10.00')
	{
		return new VC2010Project(@_);
	}
	elsif ($visualStudioVersion eq '11.00')
	{
		return new VC2012Project(@_);
	}
	elsif ($visualStudioVersion eq '12.00')
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
	my $nmakeVersion = shift;

	if (!defined($nmakeVersion))
	{

# Determine version of nmake command, to set proper version of visual studio
# we use nmake as it has existed for a long time and still exists in current visual studio versions
		open(P, "nmake /? 2>&1 |")
		  || croak
"Unable to determine Visual Studio version: The nmake command wasn't found.";
		while (<P>)
		{
			chomp;
			if (/(\d+)\.(\d+)\.\d+(\.\d+)?$/)
			{
				return _GetVisualStudioVersion($1, $2);
			}
		}
		close(P);
	}
	elsif ($nmakeVersion =~ /(\d+)\.(\d+)\.\d+(\.\d+)?$/)
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
	elsif ($major < 6)
	{
		croak
"Unable to determine Visual Studio version: Visual Studio versions before 6.0 aren't supported.";
	}
	return "$major.$minor";
}

1;
